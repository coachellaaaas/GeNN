#pragma once

// GeNN code generator includes
#include "code_generator/groupMerged.h"

//----------------------------------------------------------------------------
// GeNN::CodeGenerator::NeuronUpdateGroupMerged
//----------------------------------------------------------------------------
namespace GeNN::CodeGenerator
{
class GENN_EXPORT NeuronUpdateGroupMerged : public NeuronGroupMergedBase
{
public:
    //----------------------------------------------------------------------------
    // GeNN::CodeGenerator::NeuronUpdateGroupMerged::CurrentSource
    //----------------------------------------------------------------------------
    //! Child group merged for current sources attached to this neuron update group
    class CurrentSource : public GroupMerged<CurrentSourceInternal>
    {
    public:
        CurrentSource(size_t index, const Type::TypeContext &typeContext, Transpiler::TypeChecker::EnvironmentBase &enclosingEnv,
                      const BackendBase &backend, const std::vector<std::reference_wrapper<const CurrentSourceInternal>> &groups);

        //----------------------------------------------------------------------------
        // Public API
        //----------------------------------------------------------------------------
        void generate(const BackendBase &backend, EnvironmentExternal &env, 
                      const NeuronUpdateGroupMerged &ng, const ModelSpecMerged &modelMerged) const;

        //! Update hash with child groups
        void updateHash(boost::uuids::detail::sha1 &hash) const;

        //! Should the current source parameter be implemented heterogeneously?
        bool isParamHeterogeneous(const std::string &paramName) const;

        //! Should the current source derived parameter be implemented heterogeneously?
        bool isDerivedParamHeterogeneous(const std::string &paramName) const;

    private:
        //----------------------------------------------------------------------------
        // Private API
        //----------------------------------------------------------------------------
        //! Is the parameter referenced? **YUCK** only used for hashing
        bool isParamReferenced(const std::string &paramName) const;

        //----------------------------------------------------------------------------
        // Members
        //----------------------------------------------------------------------------
        //! List of statements parsed and type-checked in constructor; and used to generate code
        Transpiler::Statement::StatementList m_InjectionStatements;

        //! Resolved types used to generate code
        Transpiler::TypeChecker::ResolvedTypeMap m_InjectionResolvedTypes;
    };

    //----------------------------------------------------------------------------
    // GeNN::CodeGenerator::NeuronUpdateGroupMerged::InSynPSM
    //----------------------------------------------------------------------------
    //! Child group merged for incoming synapse groups
    class InSynPSM : public GroupMerged<SynapseGroupInternal>
    {
    public:
        InSynPSM(size_t index, const Type::TypeContext &typeContext, Transpiler::TypeChecker::EnvironmentBase &enclosingEnv,
                 const BackendBase &backend, const std::vector<std::reference_wrapper<const SynapseGroupInternal>> &groups);

        //----------------------------------------------------------------------------
        // Public API
        //----------------------------------------------------------------------------
        void generate(const BackendBase &backend, EnvironmentExternal &env,
                      const NeuronUpdateGroupMerged &ng, const ModelSpecMerged &modelMerged) const;

        //! Update hash with child groups
        void updateHash(boost::uuids::detail::sha1 &hash) const;

        //! Should the current source parameter be implemented heterogeneously?
        bool isParamHeterogeneous(const std::string &paramName) const;

        //! Should the current source derived parameter be implemented heterogeneously?
        bool isDerivedParamHeterogeneous(const std::string &paramName) const;

    private:
        //----------------------------------------------------------------------------
        // Private API
        //----------------------------------------------------------------------------
        //! Is the parameter referenced? **YUCK** only used for hashing
        bool isParamReferenced(const std::string &paramName) const;

        //----------------------------------------------------------------------------
        // Members
        //----------------------------------------------------------------------------
        //! List of statements parsed and type-checked in constructor; and used to generate decay code
        Transpiler::Statement::StatementList m_DecayStatements;
        
        //! List of statements parsed and type-checked in constructor; and used to generate apply inputcode
        Transpiler::Statement::StatementList m_ApplyInputStatements;

        //! Resolved types used to generate decay code
        Transpiler::TypeChecker::ResolvedTypeMap m_DecayResolvedTypes;

        //! Resolved types used to generate apply input code
        Transpiler::TypeChecker::ResolvedTypeMap m_ApplyInputResolvedTypes;
    };

    //----------------------------------------------------------------------------
    // GeNN::CodeGenerator::NeuronUpdateGroupMerged::OutSynPreOutput
    //----------------------------------------------------------------------------
    //! Child group merged for outgoing synapse groups with $(addToPre) logic
    class OutSynPreOutput : public GroupMerged<SynapseGroupInternal>
    {
    public:
        OutSynPreOutput(size_t index, const Type::TypeContext &typeContext, Transpiler::TypeChecker::EnvironmentBase &enclosingEnv,
                        const BackendBase &backend, const std::vector<std::reference_wrapper<const SynapseGroupInternal>> &groups);

        //----------------------------------------------------------------------------
        // Public API
        //----------------------------------------------------------------------------
        void generate(EnvironmentExternal &env, const NeuronUpdateGroupMerged &ng, 
                      const ModelSpecMerged &modelMerged) const;
    };

    //----------------------------------------------------------------------------
    // GeNN::CodeGenerator::NeuronUpdateGroupMerged::InSynWUMPostCode
    //----------------------------------------------------------------------------
    //! Child group merged for incoming synapse groups with postsynaptic update/spike code
    class InSynWUMPostCode : public GroupMerged<SynapseGroupInternal>
    {
    public:
        InSynWUMPostCode(size_t index, const Type::TypeContext &typeContext, Transpiler::TypeChecker::EnvironmentBase &enclosingEnv,
                         const BackendBase &backend, const std::vector<std::reference_wrapper<const SynapseGroupInternal>> &groups);

        //----------------------------------------------------------------------------
        // Public API
        //----------------------------------------------------------------------------
        void generate(const BackendBase &backend, EnvironmentExternal &env, const NeuronUpdateGroupMerged &ng,
                      const ModelSpecMerged &modelMerged, bool dynamicsNotSpike) const;

        void genCopyDelayedVars(EnvironmentExternal &env, const NeuronUpdateGroupMerged &ng,
                                const ModelSpecMerged &modelMerged) const;

        //! Update hash with child groups
        void updateHash(boost::uuids::detail::sha1 &hash) const;

        //! Should the current source parameter be implemented heterogeneously?
        bool isParamHeterogeneous(const std::string &paramName) const;

        //! Should the current source derived parameter be implemented heterogeneously?
        bool isDerivedParamHeterogeneous(const std::string &paramName) const;

    private:
        //----------------------------------------------------------------------------
        // Private API
        //----------------------------------------------------------------------------
        //! Is the parameter referenced? **YUCK** only used for hashing
        bool isParamReferenced(const std::string &paramName) const;

        //----------------------------------------------------------------------------
        // Members
        //----------------------------------------------------------------------------
        //! List of statements parsed and type-checked in constructor; and used to generate dynamics code
        Transpiler::Statement::StatementList m_DynamicsStatements;

        //! List of statements parsed and type-checked in constructor; and used to generate spike code
        Transpiler::Statement::StatementList m_SpikeStatements;

        //! Resolved types used to generate dynamics code
        Transpiler::TypeChecker::ResolvedTypeMap m_DynamicsResolvedTypes;

        //! Resolved types used to generate spike code
        Transpiler::TypeChecker::ResolvedTypeMap m_SpikeResolvedTypes;
    };

    //----------------------------------------------------------------------------
    // GeNN::CodeGenerator::NeuronUpdateGroupMerged::OutSynWUMPreCode
    //----------------------------------------------------------------------------
    //! Child group merged for outgoing synapse groups with presynaptic update/spike code
    class OutSynWUMPreCode : public GroupMerged<SynapseGroupInternal>
    {
    public:
        OutSynWUMPreCode(size_t index, const Type::TypeContext &typeContext, Transpiler::TypeChecker::EnvironmentBase &enclosingEnv,
                         const BackendBase &backend, const std::vector<std::reference_wrapper<const SynapseGroupInternal>> &groups);

        //----------------------------------------------------------------------------
        // Public API
        //----------------------------------------------------------------------------
        void generate(const BackendBase &backend, EnvironmentExternal &env, const NeuronUpdateGroupMerged &ng,
                      const ModelSpecMerged &modelMerged, bool dynamicsNotSpike) const;

        void genCopyDelayedVars(EnvironmentExternal &env, const NeuronUpdateGroupMerged &ng,
                                const ModelSpecMerged &modelMerged) const;

        //! Update hash with child groups
        void updateHash(boost::uuids::detail::sha1 &hash) const;

        //! Should the current source parameter be implemented heterogeneously?
        bool isParamHeterogeneous(const std::string &paramName) const;

        //! Should the current source derived parameter be implemented heterogeneously?
        bool isDerivedParamHeterogeneous(const std::string &paramName) const;

    private:
        //----------------------------------------------------------------------------
        // Private API
        //----------------------------------------------------------------------------
        //! Is the parameter referenced? **YUCK** only used for hashing
        bool isParamReferenced(const std::string &paramName) const;

        //----------------------------------------------------------------------------
        // Members
        //----------------------------------------------------------------------------
        //! List of statements parsed and type-checked in constructor; and used to generate dynamics code
        Transpiler::Statement::StatementList m_DynamicsStatements;

        //! List of statements parsed and type-checked in constructor; and used to generate spike code
        Transpiler::Statement::StatementList m_SpikeStatements;

        //! Resolved types used to generate dynamics code
        Transpiler::TypeChecker::ResolvedTypeMap m_DynamicsResolvedTypes;

        //! Resolved types used to generate spike code
        Transpiler::TypeChecker::ResolvedTypeMap m_SpikeResolvedTypes;
    };

    NeuronUpdateGroupMerged(size_t index, const Type::TypeContext &typeContext, const BackendBase &backend,
                            const std::vector<std::reference_wrapper<const NeuronGroupInternal>> &groups);

    //------------------------------------------------------------------------
    // Public API
    //------------------------------------------------------------------------
    //! Get hash digest used for detecting changes
    boost::uuids::detail::sha1::digest_type getHashDigest() const;

    void generateRunner(const BackendBase &backend, 
                        CodeStream &definitionsInternal, CodeStream &definitionsInternalFunc, 
                        CodeStream &definitionsInternalVar, CodeStream &runnerVarDecl, 
                        CodeStream &runnerMergedStructAlloc) const
    {
        generateRunnerBase(backend, definitionsInternal, definitionsInternalFunc, definitionsInternalVar,
                           runnerVarDecl, runnerMergedStructAlloc, name);
    }
    
    void generateNeuronUpdate(const BackendBase &backend, EnvironmentExternal &env, const ModelSpecMerged &modelMerged,
                              BackendBase::GroupHandlerEnv<NeuronUpdateGroupMerged> genEmitTrueSpike,
                              BackendBase::GroupHandlerEnv<NeuronUpdateGroupMerged> genEmitSpikeLikeEvent) const;
    
    void generateWUVarUpdate(const BackendBase &backend, EnvironmentExternal &env, const ModelSpecMerged &modelMerged) const;
    
    std::string getVarIndex(unsigned int batchSize, VarAccessDuplication varDuplication, const std::string &index) const;
    std::string getReadVarIndex(bool delay, unsigned int batchSize, VarAccessDuplication varDuplication, const std::string &index) const;
    std::string getWriteVarIndex(bool delay, unsigned int batchSize, VarAccessDuplication varDuplication, const std::string &index) const;

    const std::vector<CurrentSource> &getMergedCurrentSourceGroups() const { return m_MergedCurrentSourceGroups; }
    const std::vector<InSynPSM> &getMergedInSynPSMGroups() const { return m_MergedInSynPSMGroups; }
    const std::vector<OutSynPreOutput> &getMergedOutSynPreOutputGroups() const { return m_MergedOutSynPreOutputGroups; }
    const std::vector<InSynWUMPostCode> &getMergedInSynWUMPostCodeGroups() const { return m_MergedInSynWUMPostCodeGroups; }
    const std::vector<OutSynWUMPreCode> &getMergedOutSynWUMPreCodeGroups() const { return m_MergedOutSynWUMPreCodeGroups; }

    //----------------------------------------------------------------------------
    // Static constants
    //----------------------------------------------------------------------------
    static const std::string name;

private:
    //------------------------------------------------------------------------
    // Members
    //------------------------------------------------------------------------
    std::vector<CurrentSource> m_MergedCurrentSourceGroups;
    std::vector<InSynPSM> m_MergedInSynPSMGroups;
    std::vector<OutSynPreOutput> m_MergedOutSynPreOutputGroups;
    std::vector<InSynWUMPostCode> m_MergedInSynWUMPostCodeGroups;
    std::vector<OutSynWUMPreCode> m_MergedOutSynWUMPreCodeGroups;

    //! List of statements parsed and type-checked in constructor; and used to generate sim code
    Transpiler::Statement::StatementList m_SimStatements;

    //! Expression parsed and type-checked in constructor; and used to generate threshold condition code
    Transpiler::Expression::ExpressionPtr m_ThresholdConditionExpression;

    //! List of statements parsed and type-checked in constructor; and used to generate reset code
    Transpiler::Statement::StatementList m_ResetStatements;

    //! Resolved types used to generate sim code
    Transpiler::TypeChecker::ResolvedTypeMap m_SimResolvedTypes;

    //! Resolved types used to generate threshold condition code
    Transpiler::TypeChecker::ResolvedTypeMap m_ThresholdConditionResolvedTypes;

    //! Resolved types used to generate threshold condition code
    Transpiler::TypeChecker::ResolvedTypeMap m_ResetResolvedTypes;
};
}   // namespace GeNN::CodeGenerator
