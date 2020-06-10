#pragma once

// Standard C++ includes
#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <numeric>
#include <string>
#include <unordered_set>
#include <regex>

// OpenCL includes
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.hpp>

// GeNN includes
#include "backendExport.h"

// GeNN code generator includes
#include "code_generator/backendBase.h"
#include "code_generator/codeStream.h"
#include "code_generator/substitutions.h"

// OpenCL backend includes
#include "presynapticUpdateStrategy.h"

// Forward declarations
namespace filesystem
{
    class path;
}

//--------------------------------------------------------------------------
// CodeGenerator::OpenCL::DeviceSelectMethod
//--------------------------------------------------------------------------
namespace CodeGenerator
{
namespace OpenCL
{
//! Methods for selecting OpenCL platform
enum class PlatformSelect
{
    MANUAL,         //!< Use platform specified by user
};

//! Methods for selecting OpenCL device
enum class DeviceSelect
{
    MOST_MEMORY,    //!< Pick device with most global memory
    MANUAL,         //!< Use device specified by user
};

//--------------------------------------------------------------------------
// CodeGenerator::OpenCL::WorkGroupSizeSelect
//--------------------------------------------------------------------------
//! Methods for selecting OpenCL kernel workgroup size
enum class WorkGroupSizeSelect
{
    MANUAL,     //!< Use workgroup sizes specified by user
};

//--------------------------------------------------------------------------
// Kernel
//--------------------------------------------------------------------------
//! Kernels generated by OpenCL backend
enum Kernel
{
    KernelNeuronUpdate,
    KernelPresynapticUpdate,
    KernelPostsynapticUpdate,
    KernelSynapseDynamicsUpdate,
    KernelInitialize,
    KernelInitializeSparse,
    KernelPreNeuronReset,
    KernelPreSynapseReset,
    KernelMax
};

//--------------------------------------------------------------------------
// Type definitions
//--------------------------------------------------------------------------
//! Array of workgroup sizes for each kernel
using KernelWorkGroupSize = std::array<size_t, KernelMax>;

//--------------------------------------------------------------------------
// CodeGenerator::OpenCL::Preferences
//--------------------------------------------------------------------------
//! Preferences for OpenCL backend
struct Preferences : public PreferencesBase
{
    Preferences()
    {
        std::fill(manualWorkGroupSizes.begin(), manualWorkGroupSizes.end(), 32);
    }

    //! How to select OpenCL platform
    PlatformSelect platformSelectMethod = PlatformSelect::MANUAL;

    //! If platform select method is set to PlatformSelect::MANUAL, id of platform to use
    unsigned int manualPlatformID = 0;

    //! How to select OpenCL device
    DeviceSelect deviceSelectMethod = DeviceSelect::MOST_MEMORY;

    //! If device select method is set to DeviceSelect::MANUAL, id of device to use
    unsigned int manualDeviceID = 0;

    //! How to select OpenCL workgroup size
    WorkGroupSizeSelect workGroupSizeSelectMethod = WorkGroupSizeSelect::MANUAL;

    //! If block size select method is set to BlockSizeSelect::MANUAL, block size to use for each kernel
    KernelWorkGroupSize manualWorkGroupSizes;
};

//--------------------------------------------------------------------------
// CodeGenerator::OpenCL::Backend
//--------------------------------------------------------------------------
class BACKEND_EXPORT Backend : public BackendBase
{
public:
    Backend(const KernelWorkGroupSize& kernelWorkGroupSizes, const Preferences& preferences,
            const std::string& scalarType, unsigned int platformIndex, unsigned int deviceIndex);

    //--------------------------------------------------------------------------
    // CodeGenerator::BackendBase:: virtuals
    //--------------------------------------------------------------------------
    virtual void genNeuronUpdate(CodeStream &os, const ModelSpecMerged &modelMerged,
                                 NeuronGroupSimHandler simHandler, NeuronUpdateGroupMergedHandler wuVarUpdateHandler,
                                 HostHandler pushEGPHandler) const override;

    virtual void genSynapseUpdate(CodeStream &os, const ModelSpecMerged &modelMerged,
                                  PresynapticUpdateGroupMergedHandler wumThreshHandler, PresynapticUpdateGroupMergedHandler wumSimHandler,
                                  PresynapticUpdateGroupMergedHandler wumEventHandler, PresynapticUpdateGroupMergedHandler wumProceduralConnectHandler,
                                  PostsynapticUpdateGroupMergedHandler postLearnHandler, SynapseDynamicsGroupMergedHandler synapseDynamicsHandler,
                                  HostHandler pushEGPHandler) const override;

    virtual void genInit(CodeStream &os, const ModelSpecMerged &modelMerged,
                         NeuronInitGroupMergedHandler localNGHandler, SynapseDenseInitGroupMergedHandler sgDenseInitHandler,
                         SynapseConnectivityInitMergedGroupHandler sgSparseConnectHandler, SynapseSparseInitGroupMergedHandler sgSparseInitHandler,
                         HostHandler initPushEGPHandler, HostHandler initSparsePushEGPHandler) const override;

    //! Gets the stride used to access synaptic matrix rows, taking into account sparse data structure, padding etc
    virtual size_t getSynapticMatrixRowStride(const SynapseGroupInternal &sg) const override;

    virtual void genDefinitionsPreamble(CodeStream& os, const ModelSpecMerged &modelMerged) const override;
    virtual void genDefinitionsInternalPreamble(CodeStream& os, const ModelSpecMerged &modelMerged) const override;
    virtual void genRunnerPreamble(CodeStream& os, const ModelSpecMerged &modelMerged) const override;
    virtual void genAllocateMemPreamble(CodeStream& os, const ModelSpecMerged &modelMerged) const override;
    virtual void genStepTimeFinalisePreamble(CodeStream& os, const ModelSpecMerged &modelMergedl) const override;

    virtual void genVariableDefinition(CodeStream& definitions, CodeStream& definitionsInternal, const std::string& type, const std::string& name, VarLocation loc) const override;
    virtual void genVariableImplementation(CodeStream& os, const std::string& type, const std::string& name, VarLocation loc) const override;
    virtual MemAlloc genVariableAllocation(CodeStream& os, const std::string& type, const std::string& name, VarLocation loc, size_t count) const override;
    virtual void genVariableFree(CodeStream& os, const std::string& name, VarLocation loc) const override;

    virtual void genExtraGlobalParamDefinition(CodeStream& definitions, const std::string& type, const std::string& name, VarLocation loc) const override;
    virtual void genExtraGlobalParamImplementation(CodeStream& os, const std::string& type, const std::string& name, VarLocation loc) const override;
    virtual void genExtraGlobalParamAllocation(CodeStream &os, const std::string &type, const std::string &name,
                                               VarLocation loc, const std::string &countVarName = "count", const std::string &prefix = "") const override;
    virtual void genExtraGlobalParamPush(CodeStream &os, const std::string &type, const std::string &name,
                                         VarLocation loc, const std::string &countVarName = "count", const std::string &prefix = "") const override;
    virtual void genExtraGlobalParamPull(CodeStream &os, const std::string &type, const std::string &name,
                                         VarLocation loc, const std::string &countVarName = "count", const std::string &prefix = "") const override;

    //! Generate code for declaring merged group data to the 'device'
    virtual void genMergedGroupImplementation(CodeStream &os, const std::string &memorySpace, const std::string &suffix,
                                              size_t idx, size_t numGroups) const override;

    //! Generate code for pushing merged group data to the 'device'
    virtual void genMergedGroupPush(CodeStream &os, const std::string &suffix, size_t idx, size_t numGroups) const override;

    //! Generate code for pushing an updated EGP value into the merged group structure on 'device'
    virtual void genMergedExtraGlobalParamPush(CodeStream &os, const std::string &suffix, size_t mergedGroupIdx,
                                               const std::string &groupIdx, const std::string &fieldName,
                                               const std::string &egpName) const override;

    virtual void genPopVariableInit(CodeStream& os, const Substitutions& kernelSubs, Handler handler) const override;
    virtual void genVariableInit(CodeStream &os, const std::string &count, const std::string &indexVarName,
                                 const Substitutions &kernelSubs, Handler handler) const override;
    virtual void genSynapseVariableRowInit(CodeStream &os, const SynapseGroupMergedBase &sg,
                                           const Substitutions &kernelSubs, Handler handler) const override;

    virtual void genVariablePush(CodeStream& os, const std::string& type, const std::string& name, VarLocation loc, bool autoInitialized, size_t count) const override;
    virtual void genVariablePull(CodeStream& os, const std::string& type, const std::string& name, VarLocation loc, size_t count) const override;

    virtual void genCurrentVariablePush(CodeStream& os, const NeuronGroupInternal& ng, const std::string& type, const std::string& name, VarLocation loc) const override;
    virtual void genCurrentVariablePull(CodeStream& os, const NeuronGroupInternal& ng, const std::string& type, const std::string& name, VarLocation loc) const override;

    virtual void genCurrentTrueSpikePush(CodeStream& os, const NeuronGroupInternal& ng) const override
    {
        genCurrentSpikePush(os, ng, false);
    }
    virtual void genCurrentTrueSpikePull(CodeStream& os, const NeuronGroupInternal& ng) const override
    {
        genCurrentSpikePull(os, ng, false);
    }
    virtual void genCurrentSpikeLikeEventPush(CodeStream& os, const NeuronGroupInternal& ng) const override
    {
        genCurrentSpikePush(os, ng, true);
    }
    virtual void genCurrentSpikeLikeEventPull(CodeStream& os, const NeuronGroupInternal& ng) const override
    {
        genCurrentSpikePull(os, ng, true);
    }

    virtual MemAlloc genGlobalDeviceRNG(CodeStream &definitions, CodeStream &definitionsInternal, CodeStream &runner, CodeStream &allocations, CodeStream &free) const override;
    virtual MemAlloc genPopulationRNG(CodeStream& definitions, CodeStream& definitionsInternal, CodeStream& runner,
                                      CodeStream& allocations, CodeStream& free, const std::string& name, size_t count) const override;
    virtual void genTimer(CodeStream& definitions, CodeStream& definitionsInternal, CodeStream& runner,
                          CodeStream& allocations, CodeStream& free, CodeStream& stepTimeFinalise,
                          const std::string& name, bool updateInStepTime) const override;

    //! Generate code to return amount of free 'device' memory in bytes
    virtual void genReturnFreeDeviceMemoryBytes(CodeStream &os) const override;

    virtual void genMakefilePreamble(std::ostream& os) const override;
    virtual void genMakefileLinkRule(std::ostream& os) const override;
    virtual void genMakefileCompileRule(std::ostream& os) const override;

    virtual void genMSBuildConfigProperties(std::ostream& os) const override;
    virtual void genMSBuildImportProps(std::ostream& os) const override;
    virtual void genMSBuildItemDefinitions(std::ostream& os) const override;
    virtual void genMSBuildCompileModule(const std::string& moduleName, std::ostream& os) const override;
    virtual void genMSBuildImportTarget(std::ostream& os) const override;

    virtual std::string getArrayPrefix() const override { return m_Preferences.automaticCopy ? "" : "d_"; }
    virtual std::string getScalarPrefix() const override { return "d_"; }
    virtual std::string getPointerPrefix() const override { return "__global "; };

    //! Different backends use different RNGs for different things. Does this one require a global host RNG for the specified model?
    virtual bool isGlobalHostRNGRequired(const ModelSpecMerged &modelMerged) const override;

    //! Different backends use different RNGs for different things. Does this one require a global device RNG for the specified model?
    virtual bool isGlobalDeviceRNGRequired(const ModelSpecMerged &modelMerged) const override;

    virtual bool isPopulationRNGRequired() const override { return true; }
    virtual bool isSynRemapRequired() const override { return true; }
    virtual bool isPostsynapticRemapRequired() const override { return true; }

    //! Is automatic copy mode enabled in the preferences?
    virtual bool isAutomaticCopyEnabled() const override { return m_Preferences.automaticCopy; }

    //! Should GeNN generate empty state push and pull functions?
    virtual bool shouldGenerateEmptyStatePushPull() const override { return m_Preferences.generateEmptyStatePushPull; }

    //! Should GeNN generate pull functions for extra global parameters? These are very rarely used
    virtual bool shouldGenerateExtraGlobalParamPull() const override { return m_Preferences.generateExtraGlobalParamPull; }

    //! How many bytes of memory does 'device' have
    virtual size_t getDeviceMemoryBytes() const override { return m_ChosenDevice.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(); }
    
    //! Some backends will have additional small, fast, memory spaces for read-only data which might
    //! Be well-suited to storing merged group structs. This method returns the prefix required to
    //! Place arrays in these and their size in preferential order
    virtual MemorySpaces getMergedGroupMemorySpaces(const ModelSpecMerged &modelMerged) const override;

    //--------------------------------------------------------------------------
    // Public API
    //--------------------------------------------------------------------------
    const cl::Device& getChosenOpenCLDevice() const { return m_ChosenDevice; }
    
    std::string getFloatAtomicAdd(const std::string& ftype, const char* memoryType = "global") const;

    size_t getKernelBlockSize(Kernel kernel) const { return m_KernelWorkGroupSizes.at(kernel); }

    //--------------------------------------------------------------------------
    // Static API
    //--------------------------------------------------------------------------
    static size_t getNumPresynapticUpdateThreads(const SynapseGroupInternal& sg);
    static size_t getNumPostsynapticUpdateThreads(const SynapseGroupInternal& sg);
    static size_t getNumSynapseDynamicsThreads(const SynapseGroupInternal& sg);

    //! Register a new presynaptic update strategy
    /*! This function should be called with strategies in ascending order of preference */
    static void addPresynapticUpdateStrategy(PresynapticUpdateStrategy::Base* strategy);

    //--------------------------------------------------------------------------
    // Constants
    //--------------------------------------------------------------------------
    static const char* KernelNames[KernelMax];

private:
    //--------------------------------------------------------------------------
    // Type definitions
    //--------------------------------------------------------------------------
    template<typename T>
    using GetPaddedGroupSizeFunc = std::function<size_t(const T &)>;

    //--------------------------------------------------------------------------
    // Private methods
    //--------------------------------------------------------------------------
    template<typename T>
    void genParallelGroup(CodeStream &os, const Substitutions &kernelSubs, const std::vector<T> &groups, const std::string &mergedGroupPrefix, size_t &idStart,
                          GetPaddedGroupSizeFunc<typename T::GroupInternal> getPaddedSizeFunc,
                          GroupHandler<T> handler) const
    {
        // Loop through groups
        for(const auto &gMerge : groups) {
            // Sum padded sizes of each group within merged group
            const size_t paddedSize = std::accumulate(
                gMerge.getGroups().cbegin(), gMerge.getGroups().cend(), size_t{0},
                [gMerge, getPaddedSizeFunc](size_t acc, std::reference_wrapper<const typename T::GroupInternal> g)
            {
                return (acc + getPaddedSizeFunc(g.get()));
            });

            os << "// merged" << gMerge.getIndex() << std::endl;

            // If this is the first  group
            if(idStart == 0) {
                os << "if(id < " << paddedSize << ")";
            }
            else {
                os << "if(id >= " << idStart << " && id < " << idStart + paddedSize << ")";
            }
            {
                CodeStream::Scope b(os);
                Substitutions popSubs(&kernelSubs);

                if(gMerge.getGroups().size() == 1) {
                    os << "const __global struct Merged" << mergedGroupPrefix << "Group" << gMerge.getIndex() << " *group";
                    os << " = d_merged" << mergedGroupPrefix << "Group" << gMerge.getIndex() << "[0]; " << std::endl;
                    os << "const unsigned int lid = id - " << idStart << ";" << std::endl;
                }
                else {
                    // Perform bisect operation to get index of merged struct
                    os << "unsigned int lo = 0;" << std::endl;
                    os << "unsigned int hi = " << gMerge.getGroups().size() << ";" << std::endl;
                    os << "while(lo < hi)" << std::endl;
                    {
                        CodeStream::Scope b(os);
                        os << "const unsigned int mid = (lo + hi) / 2;" << std::endl;

                        os << "if(id < d_merged" << mergedGroupPrefix << "GroupStartID" << gMerge.getIndex() << "[mid])";
                        {
                            CodeStream::Scope b(os);
                            os << "hi = mid;" << std::endl;
                        }
                        os << "else";
                        {
                            CodeStream::Scope b(os);
                            os << "lo = mid + 1;" << std::endl;
                        }
                    }

                    // Use this to get reference to merged group structure
                    os << "const __global struct Merged" << mergedGroupPrefix << "Group" << gMerge.getIndex() << " *group";
                    os << " = &d_merged" << mergedGroupPrefix << "Group" << gMerge.getIndex() << "[lo - 1]; " << std::endl;

                    // Use this and starting thread of merged group to calculate local id within neuron group
                    os << "const unsigned int lid = id - (d_merged" << mergedGroupPrefix << "GroupStartID" << gMerge.getIndex() << "[lo - 1]);" << std::endl;

                }
                popSubs.addVarSubstitution("id", "lid");
                handler(os, gMerge, popSubs);

                idStart += paddedSize;
            }
        }
    }

    void genEmitSpike(CodeStream& os, const Substitutions& subs, const std::string& suffix) const;

    void genCurrentSpikePush(CodeStream& os, const NeuronGroupInternal& ng, bool spikeEvent) const;
    void genCurrentSpikePull(CodeStream& os, const NeuronGroupInternal& ng, bool spikeEvent) const;

    void genKernelDimensions(CodeStream& os, Kernel kernel, size_t numThreads) const;

    //! Adds a type - both to backend base's list of sized types but also to device types set
    void addDeviceType(const std::string& type, size_t size);

    //! Is type a a device only type?
    bool isDeviceType(const std::string& type) const;

    void divideKernelStreamInParts(CodeStream& os, const std::stringstream& kernelCode, size_t partLength) const;

    //--------------------------------------------------------------------------
    // Private static methods
    //--------------------------------------------------------------------------
    // Get appropriate presynaptic update strategy to use for this synapse group
    static const PresynapticUpdateStrategy::Base* getPresynapticUpdateStrategy(const SynapseGroupInternal& sg);

    //--------------------------------------------------------------------------
    // Members
    //--------------------------------------------------------------------------
    const KernelWorkGroupSize m_KernelWorkGroupSizes;
    const Preferences m_Preferences;

    const unsigned int m_ChosenPlatformIndex;
    const unsigned int m_ChosenDeviceIndex;
    cl::Device m_ChosenDevice;

    int m_RuntimeVersion;

    //! Types that are only supported on device i.e. should never be exposed to user code
    std::unordered_set<std::string> m_DeviceTypes;

    //--------------------------------------------------------------------------
    // Static members
    //--------------------------------------------------------------------------
    static std::vector<PresynapticUpdateStrategy::Base*> s_PresynapticUpdateStrategies;
};
}   // OpenCL
}   // CodeGenerator
