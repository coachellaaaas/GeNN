// Google test includes
#include "gtest/gtest.h"

// GeNN includes
#include "type.h"

// GeNN transpiler includes
#include "transpiler/errorHandler.h"
#include "transpiler/parser.h"
#include "transpiler/scanner.h"
#include "transpiler/standardLibrary.h"
#include "transpiler/typeChecker.h"

using namespace GeNN;
using namespace GeNN::Transpiler;

//--------------------------------------------------------------------------
// Anonymous namespace
//--------------------------------------------------------------------------
namespace
{
class TestErrorHandler : public ErrorHandlerBase
{
public:
    TestErrorHandler() : m_Error(false)
    {}

    bool hasError() const { return m_Error; }

    virtual void error(size_t line, std::string_view message) override
    {
        report(line, "", message);
    }

    virtual void error(const Token &token, std::string_view message) override
    {
        if(token.type == Token::Type::END_OF_FILE) {
            report(token.line, " at end", message);
        }
        else {
            report(token.line, " at '" + token.lexeme + "'", message);
        }
    }

private:
    void report(size_t line, std::string_view where, std::string_view message)
    {
        std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
        m_Error = true;
    }

    bool m_Error;
};

class TestEnvironment : public TypeChecker::EnvironmentBase
{
public:
    //---------------------------------------------------------------------------
    // Public API
    //---------------------------------------------------------------------------
    void define(const Type::ResolvedType &type, const std::string &name, Type::Qualifier qualifiers = Type::Qualifier{0})
    {
        if(!m_Types.try_emplace(name, type.addQualifier(qualifiers)).second) {
            throw std::runtime_error("Redeclaration of '" + std::string{name} + "'");
        }
    }


    void definePointer(const Type::ResolvedType &type, const std::string &name, Type::Qualifier valueQualifiers = Type::Qualifier{0}, 
                       Type::Qualifier pointerQualifiers = Type::Qualifier{0})
    {
        define(type.addQualifier(valueQualifiers).createPointer(pointerQualifiers), name);
    }


    //---------------------------------------------------------------------------
    // EnvironmentBase virtuals
    //---------------------------------------------------------------------------
    virtual void define(const Token &name, const Type::ResolvedType&, ErrorHandlerBase &errorHandler) final
    {
        errorHandler.error(name, "Cannot declare variable in external environment");
        throw TypeChecker::TypeCheckError();
    }

    virtual std::vector<Type::ResolvedType> getTypes(const Token &name, ErrorHandlerBase &errorHandler) final
    {
        auto type = m_Types.find(std::string{name.lexeme});
        if(type == m_Types.end()) {
            errorHandler.error(name, "Undefined variable");
            throw TypeChecker::TypeCheckError();
        }
        else {
            return {type->second};
        }
    }

private:
    //---------------------------------------------------------------------------
    // Members
    //---------------------------------------------------------------------------
    std::unordered_map<std::string, Type::ResolvedType> m_Types;
};

void typeCheckStatements(std::string_view code, TypeChecker::EnvironmentBase &typeEnvironment, const Type::TypeContext &typeContext = {})
{
    // Scan
    TestErrorHandler errorHandler;
    const auto tokens = Scanner::scanSource(code, typeContext, errorHandler);
    ASSERT_FALSE(errorHandler.hasError());
 
    // Parse
    const auto statements = Parser::parseBlockItemList(tokens, typeContext, errorHandler);
    ASSERT_FALSE(errorHandler.hasError());

    // Typecheck
    TypeChecker::typeCheck(statements, typeEnvironment, errorHandler);
    ASSERT_FALSE(errorHandler.hasError());
}

Type::ResolvedType typeCheckExpression(std::string_view code, TypeChecker::EnvironmentBase &typeEnvironment, const Type::TypeContext &typeContext = {})
{
    // Scan
    TestErrorHandler errorHandler;
    const auto tokens = Scanner::scanSource(code, typeContext, errorHandler);
    EXPECT_FALSE(errorHandler.hasError());
 
    // Parse
    const auto expression = Parser::parseExpression(tokens, typeContext, errorHandler);
    EXPECT_FALSE(errorHandler.hasError());
    
    // Typecheck
    const auto type = TypeChecker::typeCheck(expression.get(), typeEnvironment, errorHandler);
    EXPECT_FALSE(errorHandler.hasError());
    return type;
}
}   // Anonymous namespace

//--------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------
TEST(TypeChecker, ArraySubscript)
{
    // Integer array indexing
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        const auto type = typeCheckExpression("intArray[4]", typeEnvironment);
        EXPECT_EQ(type, Type::Int32);
        EXPECT_FALSE(type.hasQualifier(Type::Qualifier::CONSTANT));
    }

    // Pointer to pointer, double indexing

    // Float array indexing
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeCheckExpression("intArray[4.0f]", typeEnvironment);}, 
        TypeChecker::TypeCheckError);

    // Pointer indexing
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeEnvironment.definePointer(Type::Int32, "indexArray");
        typeCheckExpression("intArray[indexArray]", typeEnvironment);}, 
        TypeChecker::TypeCheckError);
}
//--------------------------------------------------------------------------
TEST(TypeChecker, Assignment)
{
    // Numeric assignment
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal");
        typeEnvironment.define(Type::Float, "floatVal");
        typeEnvironment.define(Type::Int32, "intValConst", Type::Qualifier::CONSTANT);
        typeCheckStatements(
            "int w = intVal;\n"
            "float x = floatVal;\n"
            "int y = floatVal;\n"
            "float z = intVal;\n"
            "int wc = intValConst;\n"
            "const int cw = intVal;\n"
            "const int cwc = intValConst;\n",
            typeEnvironment);
    }

    // Pointer assignement
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeEnvironment.definePointer(Type::Int32, "intArrayConst", Type::Qualifier::CONSTANT);
        typeCheckStatements(
            "int *x = intArray;\n"
            "const int *y = intArray;\n"
            "const int *z = intArrayConst;\n", 
            typeEnvironment);
    }

    // Pointer assignement, attempt to remove const
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray", Type::Qualifier::CONSTANT);
        typeCheckStatements("int *x = intArray;", typeEnvironment);},
        TypeChecker::TypeCheckError);

    // Pointer assignement without explicit cast
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeCheckStatements("float *x = intArray;", typeEnvironment);},
        TypeChecker::TypeCheckError);

    // Dereference assignment
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeCheckStatements(
            "*intArray = 7;\n",
            typeEnvironment);
    }
    // **TODO** other assignements i.e. += -= %=
}
//--------------------------------------------------------------------------
TEST(TypeChecker, Binary)
{
    // Pointer difference
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray1");
        typeEnvironment.definePointer(Type::Int32, "intArray2");
        const auto type = typeCheckExpression("intArray1 - intArray2", typeEnvironment);
        EXPECT_EQ(type, Type::Int32);
    }

    // **TODO** different pointer types


    // Pointer + integer
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeEnvironment.define(Type::Int32, "offset");
        const auto type = typeCheckExpression("intArray + offset", typeEnvironment);
        EXPECT_EQ(*type.getPointer().valueType, Type::Int32);       
    }

    // **TODO** constness and 

    // Pointer + non-integer
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeEnvironment.define(Type::Float, "offset");
        typeCheckExpression("intArray + offset", typeEnvironment);},
        TypeChecker::TypeCheckError);

    // Pointer + pointer
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray1");
        typeEnvironment.definePointer(Type::Int32, "intArray2");
        typeCheckExpression("intArray1 + intArray2", typeEnvironment);},
        TypeChecker::TypeCheckError);


    // Pointer - integer
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeEnvironment.define(Type::Int32, "offset");
        const auto type = typeCheckExpression("intArray - offset", typeEnvironment);
        EXPECT_EQ(*type.getPointer().valueType, Type::Int32);
    }

    // Integer + pointer
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeEnvironment.define(Type::Int32, "offset");
        const auto type = typeCheckExpression("offset + intArray", typeEnvironment);
        EXPECT_EQ(*type.getPointer().valueType, Type::Int32);       
    }

    /*integer only (opType == Token::Type::PERCENT || opType == Token::Type::SHIFT_LEFT
                    || opType == Token::Type::SHIFT_RIGHT || opType == Token::Type::CARET
                    || opType == Token::Type::AMPERSAND || opType == Token::Type::PIPE)*/

}
//--------------------------------------------------------------------------
TEST(TypeChecker, Call)
{
    // Too few arguments
    StandardLibrary::FunctionTypes stdLibraryEnv;
    EXPECT_THROW({
        typeCheckExpression("sin()", stdLibraryEnv);}, 
        TypeChecker::TypeCheckError);

    // Too many arguments
    EXPECT_THROW({
        typeCheckExpression("sin(1.0f, 2.0f)", stdLibraryEnv);}, 
        TypeChecker::TypeCheckError);

    // Floating point transcendental function
    {
        const auto type = typeCheckExpression("sin(1.0f)", stdLibraryEnv);
        EXPECT_EQ(type, Type::Float);
    }

    // Double transcendental function
    {
        const auto type = typeCheckExpression("sin(1.0d)", stdLibraryEnv);
        EXPECT_EQ(type, Type::Double);
    }

    // Float scalar transcendental function
    {
        const Type::TypeContext typeContext{{"scalar", Type::Float}};
        const auto type = typeCheckExpression("sin(1.0)", stdLibraryEnv, typeContext);
        EXPECT_EQ(type, Type::Float);
    }

    // Double scalar transcendental function
    {
        const Type::TypeContext typeContext{{"scalar", Type::Double}};
        const auto type = typeCheckExpression("sin(1.0)", stdLibraryEnv, typeContext);
        EXPECT_EQ(type, Type::Double);
    }

    // Nested transcendental function
    {
        const auto type = typeCheckExpression("sin(fmax(0.0f, 1.0f))", stdLibraryEnv);
        EXPECT_EQ(type, Type::Float);
    }


    // Variadic with too few arguments
    /*EXPECT_THROW({
        typeCheckExpression("printf()", stdLibraryEnv);},
        TypeChecker::TypeCheckError);

    // Variadic function with no extra arguments
    {
        const auto *type = typeCheckExpression("printf(\"hello world\")", stdLibraryEnv);
        EXPECT_EQ(type->getName(), Type::Int32::getInstance()->getName());
    }

    // Variadic function with extra arguments
    {
        const auto *type = typeCheckExpression("printf(\"hello world %d, %f\", 12, cos(5.0f))", stdLibraryEnv);
        EXPECT_EQ(type->getName(), Type::Int32::getInstance()->getName());
    }*/

}
//--------------------------------------------------------------------------
TEST(TypeChecker, Cast)
{
    // Numeric cast
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal");
        const auto type = typeCheckExpression("(float)intVal", typeEnvironment);
        EXPECT_EQ(type, Type::Float);
    }

    // Numeric cast to const
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal");
        const auto type = typeCheckExpression("(const int)intVal", typeEnvironment);
        EXPECT_EQ(type, Type::Int32.addQualifier(Type::Qualifier::CONSTANT));
    }

    // Pointer cast to value const
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        const auto type = typeCheckExpression("(const int*)intArray", typeEnvironment);
        EXPECT_FALSE(type.hasQualifier(Type::Qualifier::CONSTANT));

        EXPECT_EQ(*type.getPointer().valueType, Type::Int32.addQualifier(Type::Qualifier::CONSTANT));
    }

    // Pointer cast to pointer const
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        const auto type = typeCheckExpression("(int * const)intArray", typeEnvironment);
        EXPECT_TRUE(type.hasQualifier(Type::Qualifier::CONSTANT));
        EXPECT_EQ(*type.getPointer().valueType, Type::Int32);    
    }

    // Can't remove value const from numeric
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal", Type::Qualifier::CONSTANT);
        typeCheckExpression("(int)intVal", typeEnvironment);},
        TypeChecker::TypeCheckError);

    // Can't remove value const from pointer
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray", Type::Qualifier::CONSTANT);
        typeCheckExpression("(int*)intArray", typeEnvironment);},
        TypeChecker::TypeCheckError);

    // Can't remove pointer const from pointer
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray", Type::Qualifier{0}, Type::Qualifier::CONSTANT);
        typeCheckExpression("(int*)intArray", typeEnvironment);},
        TypeChecker::TypeCheckError);

   // Pointer cast can't reinterpret
   EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeCheckExpression("(float*)intArray", typeEnvironment);},
        TypeChecker::TypeCheckError);
    
   // Pointer can't be cast to numeric
   EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeCheckExpression("(int)intArray", typeEnvironment);},
        TypeChecker::TypeCheckError);
    
   // Numeric can't be cast to pointer
   EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal");
        typeCheckExpression("(int*)intVal", typeEnvironment);},
        TypeChecker::TypeCheckError);
}
//--------------------------------------------------------------------------
TEST(TypeChecker, Conditional)
{
}
//--------------------------------------------------------------------------
TEST(TypeChecker, IncDec)
{
    // Can increment numeric
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal");
        const auto type = typeCheckExpression("intVal++", typeEnvironment);
        EXPECT_EQ(type, Type::Int32);
        EXPECT_FALSE(type.hasQualifier(Type::Qualifier::CONSTANT));
    }

    // Can increment pointer
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        const auto type = typeCheckExpression("intArray++", typeEnvironment);
        EXPECT_EQ(type, Type::Int32.createPointer());
        EXPECT_FALSE(type.hasQualifier(Type::Qualifier::CONSTANT));
    }

    // Can increment pointer to const
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray", Type::Qualifier::CONSTANT);
        const auto type = typeCheckExpression("intArray++", typeEnvironment);
        EXPECT_FALSE(type.hasQualifier(Type::Qualifier::CONSTANT));
        EXPECT_EQ(*type.getPointer().valueType, Type::Int32.addQualifier(Type::Qualifier::CONSTANT));
    }

   // Can't increment const number
   EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal", Type::Qualifier::CONSTANT);
        typeCheckExpression("intVal++", typeEnvironment);},
        TypeChecker::TypeCheckError);
   
   // Can't increment const pointer
   EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray", Type::Qualifier{0}, Type::Qualifier::CONSTANT);
        typeCheckExpression("intArray++", typeEnvironment);},
        TypeChecker::TypeCheckError);
}
//--------------------------------------------------------------------------
TEST(TypeChecker, Literal)
{
    // Float
    {
        TestEnvironment typeEnvironment;
        const auto type = typeCheckExpression("1.0f", typeEnvironment);
        EXPECT_EQ(type, Type::Float);
    }

    // Scalar with single-precision
    {
        TestEnvironment typeEnvironment;
        const Type::TypeContext typeContext{{"scalar", Type::Float}};
        const auto type = typeCheckExpression("1.0", typeEnvironment, typeContext);
        EXPECT_EQ(type, Type::Float);
    }

    // Scalar with double-precision
    {
        TestEnvironment typeEnvironment;
        const Type::TypeContext typeContext{{"scalar", Type::Double}};
        const auto type = typeCheckExpression("1.0", typeEnvironment, typeContext);
        EXPECT_EQ(type, Type::Double);
    }

    // Double
    {
        TestEnvironment typeEnvironment;
        const auto type = typeCheckExpression("1.0d", typeEnvironment);
        EXPECT_EQ(type, Type::Double);
    }

    // Integer
    {
        TestEnvironment typeEnvironment;
        const auto type = typeCheckExpression("100", typeEnvironment);
        EXPECT_EQ(type, Type::Int32);
    }

    // Unsigned integer
    {
        TestEnvironment typeEnvironment;
        const auto type = typeCheckExpression("100U", typeEnvironment);
        EXPECT_EQ(type, Type::Uint32);
    }

    // String
    {
        TestEnvironment typeEnvironment;
        const auto type = typeCheckExpression("\"hello world\"", typeEnvironment);
        EXPECT_EQ(type, Type::Int8.createPointer(Type::Qualifier::CONSTANT));
    }
}
//--------------------------------------------------------------------------
TEST(TypeChecker, Unary)
{
    // Dereference pointer
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        const auto type = typeCheckExpression("*intArray", typeEnvironment);
        EXPECT_EQ(type, Type::Int32);
    }

    // Dereference pointer to const
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray", Type::Qualifier::CONSTANT);
        const auto type = typeCheckExpression("*intArray", typeEnvironment);
        EXPECT_EQ(type, Type::Int32.addQualifier(Type::Qualifier::CONSTANT));
    }

    // Dereference const pointer
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray", Type::Qualifier{0}, Type::Qualifier::CONSTANT);
        const auto type = typeCheckExpression("*intArray", typeEnvironment);
        EXPECT_EQ(type, Type::Int32);
    }

    // Dereference const pointer to const
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray", Type::Qualifier::CONSTANT, Type::Qualifier::CONSTANT);
        const auto type = typeCheckExpression("*intArray", typeEnvironment);
        EXPECT_EQ(type, Type::Int32.addQualifier(Type::Qualifier::CONSTANT));
    }

    // Dereference numeric
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal");
        typeCheckExpression("*intVal", typeEnvironment); },
        TypeChecker::TypeCheckError);

    // Address of numeric
    {
        TestEnvironment typeEnvironment;
        typeEnvironment.define(Type::Int32, "intVal");
        const auto type = typeCheckExpression("&intVal", typeEnvironment);
        EXPECT_FALSE(type.hasQualifier(Type::Qualifier::CONSTANT));
        EXPECT_EQ(*type.getPointer().valueType, Type::Int32);
        EXPECT_FALSE(type.getPointer().valueType->hasQualifier(Type::Qualifier::CONSTANT));        
    }

    // Address of pointer
    EXPECT_THROW({
        TestEnvironment typeEnvironment;
        typeEnvironment.definePointer(Type::Int32, "intArray");
        typeCheckExpression("&intArray", typeEnvironment);},
        TypeChecker::TypeCheckError);
}
