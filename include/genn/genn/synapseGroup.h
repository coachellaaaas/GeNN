#pragma once

// Standard includes
#include <map>
#include <string>
#include <vector>

// GeNN includes
#include "gennExport.h"
#include "initSparseConnectivitySnippet.h"
#include "postsynapticModels.h"
#include "weightUpdateModels.h"
#include "synapseMatrixType.h"
#include "variableMode.h"

// Forward declarations
class NeuronGroupInternal;
class SynapseGroupInternal;

//------------------------------------------------------------------------
// SynapseGroup
//------------------------------------------------------------------------
class GENN_EXPORT SynapseGroup
{
public:
    SynapseGroup(const SynapseGroup&) = delete;
    SynapseGroup() = delete;

    //------------------------------------------------------------------------
    // Enumerations
    //------------------------------------------------------------------------
    enum class SpanType
    {
        POSTSYNAPTIC,
        PRESYNAPTIC
    };

    //------------------------------------------------------------------------
    // Public methods
    //------------------------------------------------------------------------
    //! Set location of weight update model state variable
    /*! This is ignored for simulations on hardware with a single memory space */
    void setWUVarLocation(const std::string &varName, VarLocation loc);

    //! Set location of weight update model presynaptic state variable
    /*! This is ignored for simulations on hardware with a single memory space */
    void setWUPreVarLocation(const std::string &varName, VarLocation loc);
    
    //! Set location of weight update model postsynaptic state variable
    /*! This is ignored for simulations on hardware with a single memory space */
    void setWUPostVarLocation(const std::string &varName, VarLocation loc);
    
    //! Set location of weight update model extra global parameter
    /*! This is ignored for simulations on hardware with a single memory space
        and only applies to extra global parameters which are pointers. */
    void setWUExtraGlobalParamLocation(const std::string &paramName, VarLocation loc);

    //! Set location of postsynaptic model state variable
    /*! This is ignored for simulations on hardware with a single memory space */
    void setPSVarLocation(const std::string &varName, VarLocation loc);

    //! Set location of postsynaptic model extra global parameter
    /*! This is ignored for simulations on hardware with a single memory space
        and only applies to extra global parameters which are pointers. */
    void setPSExtraGlobalParamLocation(const std::string &paramName, VarLocation loc);

    //! Set location of sparse connectivity initialiser extra global parameter
    /*! This is ignored for simulations on hardware with a single memory space
        and only applies to extra global parameters which are pointers. */
    void setSparseConnectivityExtraGlobalParamLocation(const std::string &paramName, VarLocation loc);

    //! Set location of variables used to combine input from this synapse group
    /*! This is ignored for simulations on hardware with a single memory space */
    void setInSynVarLocation(VarLocation loc) { m_InSynLocation = loc; }

    //! Set variable mode used for sparse connectivity
    /*! This is ignored for simulations on hardware with a single memory space */
    void setSparseConnectivityLocation(VarLocation loc);

    //! Set variable mode used for this synapse group's dendritic delay buffers
    void setDendriticDelayLocation(VarLocation loc) { m_DendriticDelayLocation = loc; }

    //! Sets the maximum number of target neurons any source neurons can connect to
    /*! Use with synaptic matrix types with SynapseMatrixConnectivity::SPARSE to optimise CUDA implementation */
    void setMaxConnections(unsigned int maxConnections);

    //! Sets the maximum number of source neurons any target neuron can connect to
    /*! Use with synaptic matrix types with SynapseMatrixConnectivity::SPARSE and postsynaptic learning to optimise CUDA implementation */
    void setMaxSourceConnections(unsigned int maxPostConnections);
    
    //! Sets the maximum dendritic delay for synapses in this synapse group
    void setMaxDendriticDelayTimesteps(unsigned int maxDendriticDelay);
    
    //! Set how CUDA implementation is parallelised
    /*! with a thread per target neuron (default) or a thread per source spike */
    void setSpanType(SpanType spanType);

    //! Set how many threads CUDA implementation uses to process each spike when span type is PRESYNAPTIC
    // **TODO** this shouldn't be in SynapseGroup - it's backend-specific
    void setNumThreadsPerSpike(unsigned int numThreadsPerSpike);

    //! Sets the number of delay steps used to delay postsynaptic spikes travelling back along dendrites to synapses
    void setBackPropDelaySteps(unsigned int timesteps);

    //! Enables or disables using narrow i.e. less than 32-bit types for sparse matrix indices
    void setNarrowSparseIndEnabled(bool enabled);

    //------------------------------------------------------------------------
    // Public const methods
    //------------------------------------------------------------------------
    const std::string &getName() const{ return m_Name; }

    SpanType getSpanType() const{ return m_SpanType; }
    unsigned int getNumThreadsPerSpike() const{ return m_NumThreadsPerSpike; }
    unsigned int getDelaySteps() const{ return m_DelaySteps; }
    unsigned int getBackPropDelaySteps() const{ return m_BackPropDelaySteps; }
    unsigned int getMaxConnections() const;
    unsigned int getMaxSourceConnections() const;
    unsigned int getMaxDendriticDelayTimesteps() const{ return m_MaxDendriticDelayTimesteps; }
    SynapseMatrixType getMatrixType() const{ return m_MatrixType; }
    const std::vector<unsigned int> &getKernelSize() const { return m_KernelSize; }

    //! Get variable mode used for variables used to combine input from this synapse group
    VarLocation getInSynLocation() const { return m_InSynLocation; }

    //! Get variable mode used for sparse connectivity
    VarLocation getSparseConnectivityLocation() const;

    //! Get variable mode used for this synapse group's dendritic delay buffers
    VarLocation getDendriticDelayLocation() const{ return m_DendriticDelayLocation; }

    //! Does synapse group need to handle 'true' spikes
    bool isTrueSpikeRequired() const;

    //! Does synapse group need to handle spike-like events
    bool isSpikeEventRequired() const;

    //! Is this synapse group a weight-sharing slave
    bool isWeightSharingSlave() const { return (getWeightSharingMaster() != nullptr); }

    const WeightUpdateModels::Base *getWUModel() const{ return m_WUModel; }

    const std::vector<double> &getWUParams() const{ return m_WUParams; }
    const std::vector<Models::VarInit> &getWUVarInitialisers() const{ return m_WUVarInitialisers; }
    const std::vector<Models::VarInit> &getWUPreVarInitialisers() const{ return m_WUPreVarInitialisers; }
    const std::vector<Models::VarInit> &getWUPostVarInitialisers() const{ return m_WUPostVarInitialisers; }
    const std::vector<double> getWUConstInitVals() const;

    const PostsynapticModels::Base *getPSModel() const{ return m_PSModel; }

    const std::vector<double> &getPSParams() const{ return m_PSParams; }
    const std::vector<Models::VarInit> &getPSVarInitialisers() const{ return m_PSVarInitialisers; }
    const std::vector<double> getPSConstInitVals() const;

    const InitSparseConnectivitySnippet::Init &getConnectivityInitialiser() const{ return m_ConnectivityInitialiser; }

    bool isZeroCopyEnabled() const;

    //! Get location of weight update model per-synapse state variable by name
    VarLocation getWUVarLocation(const std::string &var) const;

    //! Get location of weight update model per-synapse state variable by index
    VarLocation getWUVarLocation(size_t index) const;

    //! Get location of weight update model presynaptic state variable by name
    VarLocation getWUPreVarLocation(const std::string &var) const;

    //! Get location of weight update model presynaptic state variable by index
    VarLocation getWUPreVarLocation(size_t index) const{ return m_WUPreVarLocation.at(index); }

    //! Get location of weight update model postsynaptic state variable by name
    VarLocation getWUPostVarLocation(const std::string &var) const;

    //! Get location of weight update model postsynaptic state variable by index
    VarLocation getWUPostVarLocation(size_t index) const{ return m_WUPostVarLocation.at(index); }

    //! Get location of weight update model extra global parameter by name
    /*! This is only used by extra global parameters which are pointers*/
    VarLocation getWUExtraGlobalParamLocation(const std::string &paramName) const;

    //! Get location of  weight update model extra global parameter by index
    /*! This is only used by extra global parameters which are pointers*/
    VarLocation getWUExtraGlobalParamLocation(size_t index) const{ return m_WUExtraGlobalParamLocation.at(index); }

    //! Get location of postsynaptic model state variable
    VarLocation getPSVarLocation(const std::string &var) const;

    //! Get location of postsynaptic model state variable
    VarLocation getPSVarLocation(size_t index) const{ return m_PSVarLocation.at(index); }

    //! Get location of postsynaptic model extra global parameter by name
    /*! This is only used by extra global parameters which are pointers*/
    VarLocation getPSExtraGlobalParamLocation(const std::string &paramName) const;

    //! Get location of postsynaptic model extra global parameter by index
    /*! This is only used by extra global parameters which are pointers*/
    VarLocation getPSExtraGlobalParamLocation(size_t index) const{ return m_PSExtraGlobalParamLocation.at(index); }

    //! Get location of sparse connectivity initialiser extra global parameter by name
    /*! This is only used by extra global parameters which are pointers*/
    VarLocation getSparseConnectivityExtraGlobalParamLocation(const std::string &paramName) const;

    //! Get location of sparse connectivity initialiser extra global parameter by index
    /*! This is only used by extra global parameters which are pointers*/
    VarLocation getSparseConnectivityExtraGlobalParamLocation(size_t index) const;

    //! Does this synapse group require dendritic delay?
    bool isDendriticDelayRequired() const;

    //! Does this synapse group require an RNG to generate procedural connectivity?
    bool isProceduralConnectivityRNGRequired() const;

    //! Does this synapse group require an RNG for it's postsynaptic init code?
    bool isPSInitRNGRequired() const;

    //! Does this synapse group require an RNG for it's weight update init code?
    bool isWUInitRNGRequired() const;

    //! Does this synapse group require an RNG for it's weight update presynaptic variable init code?
    bool isWUPreInitRNGRequired() const;

    //! Does this synapse group require an RNG for it's weight update postsynaptic variable init code?
    bool isWUPostInitRNGRequired() const;

    //! Does this synapse group require a RNG for any sort of initialization
    bool isHostInitRNGRequired() const;

    //! Is var init code required for any variables in this synapse group's weight update model?
    bool isWUVarInitRequired() const;

    //! Is sparse connectivity initialisation code required for this synapse group?
    bool isSparseConnectivityInitRequired() const;

protected:
    SynapseGroup(const std::string &name, SynapseMatrixType matrixType, unsigned int delaySteps,
                 const WeightUpdateModels::Base *wu, const std::vector<double> &wuParams, const std::vector<Models::VarInit> &wuVarInitialisers, const std::vector<Models::VarInit> &wuPreVarInitialisers, const std::vector<Models::VarInit> &wuPostVarInitialisers,
                 const PostsynapticModels::Base *ps, const std::vector<double> &psParams, const std::vector<Models::VarInit> &psVarInitialisers,
                 NeuronGroupInternal *srcNeuronGroup, NeuronGroupInternal *trgNeuronGroup, const SynapseGroupInternal *weightSharingMaster,
                 const InitSparseConnectivitySnippet::Init &connectivityInitialiser,
                 VarLocation defaultVarLocation, VarLocation defaultExtraGlobalParamLocation,
                 VarLocation defaultSparseConnectivityLocation, bool defaultNarrowSparseIndEnabled);

    //------------------------------------------------------------------------
    // Protected methods
    //------------------------------------------------------------------------
    NeuronGroupInternal *getSrcNeuronGroup(){ return m_SrcNeuronGroup; }
    NeuronGroupInternal *getTrgNeuronGroup(){ return m_TrgNeuronGroup; }

    void setEventThresholdReTestRequired(bool req){ m_EventThresholdReTestRequired = req; }

    //! Set if any of this synapse group's weight update model variables referenced by a custom update
    void setWUVarReferencedByCustomUpdate(bool ref) { m_WUVarReferencedByCustomUpdate = ref;  }

    void setPSModelMergeTarget(const std::string &targetName)
    {
        m_PSModelTargetName = targetName;
    }
    
    void initDerivedParams(double dt);

    //------------------------------------------------------------------------
    // Protected const methods
    //------------------------------------------------------------------------
    const NeuronGroupInternal *getSrcNeuronGroup() const{ return m_SrcNeuronGroup; }
    const NeuronGroupInternal *getTrgNeuronGroup() const{ return m_TrgNeuronGroup; }

    const std::vector<double> &getWUDerivedParams() const{ return m_WUDerivedParams; }
    const std::vector<double> &getPSDerivedParams() const{ return m_PSDerivedParams; }

    const SynapseGroupInternal *getWeightSharingMaster() const { return m_WeightSharingMaster; }

    //!< Does the event threshold needs to be retested in the synapse kernel?
    /*! This is required when the pre-synaptic neuron population's outgoing synapse groups require different event threshold */
    bool isEventThresholdReTestRequired() const{ return m_EventThresholdReTestRequired; }

    const std::string &getPSModelTargetName() const{ return m_PSModelTargetName; }
    bool isPSModelMerged() const{ return m_PSModelTargetName != getName(); }

    //! Get the type to use for sparse connectivity indices for synapse group
    std::string getSparseIndType() const;

    //! Are any of this synapse group's weight update model variables referenced by a custom update
    bool areWUVarReferencedByCustomUpdate() const { return m_WUVarReferencedByCustomUpdate;  }

    //! Updates hash with weight update component of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getWUHashDigest() const;

    //! Updates hash with presynaptic update component of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getWUPreHashDigest() const;

    //! Updates hash with postsynaptic update component of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getWUPostHashDigest() const;

    //! Updates hash with postsynaptic update component of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getPSHashDigest() const;

    boost::uuids::detail::sha1::digest_type getDendriticDelayUpdateHashDigest() const;

    //! Can postsynaptic update component of this synapse group be merged with other? i.e. can they be simulated using same generated code
    /*! NOTE: this can only be called after model is finalized */
    bool canPSBeMerged(const SynapseGroup &other) const;

    //! Can postsynaptic update component of this synapse group not only be merged with other, but combined so only one needs simulating at all
    /*! NOTE: this can only be called after model is finalized */
    bool canPSBeLinearlyCombined(const SynapseGroup &other) const;

    //! Updates hash with initialisation component of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getWUInitHashDigest() const;

     //! Updates hash with presynaptic variable initialisation component of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getWUPreInitHashDigest() const;

    //! Updates hash with postsynaptic variable initialisation component of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getWUPostInitHashDigest() const;

    //! Updates hash with postsynaptic model variable initialisation component of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getPSInitHashDigest() const;

    //! Updates hash with connectivity initialisation of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getConnectivityInitHashDigest() const;

    //! Updates hash with host connectivity initialisation of this synapse group
    /*! NOTE: this can only be called after model is finalized */
    boost::uuids::detail::sha1::digest_type getConnectivityHostInitHashDigest() const;

private:
    //------------------------------------------------------------------------
    // Members
    //------------------------------------------------------------------------
    //! Name of the synapse group
    const std::string m_Name;

    //! Execution order of synapses in the kernel. It determines whether synapses are executed in parallel for every postsynaptic neuron, or for every presynaptic neuron.
    SpanType m_SpanType;

    //! How many threads CUDA implementation uses to process each spike when span type is PRESYNAPTIC
    unsigned int m_NumThreadsPerSpike;

    //! Global synaptic conductance delay for the group (in time steps)
    unsigned int m_DelaySteps;

    //! Global backpropagation delay for postsynaptic spikes to synapse (in time
    unsigned int m_BackPropDelaySteps;

    //! Maximum number of target neurons any source neuron can connect to
    unsigned int m_MaxConnections;
    
    //! Maximum number of source neurons any target neuron can connect to
    unsigned int m_MaxSourceConnections;

    //! Maximum dendritic delay timesteps supported for synapses in this population
    unsigned int m_MaxDendriticDelayTimesteps;

    //! Kernel size 
    std::vector<unsigned int> m_KernelSize;
    
    //! Connectivity type of synapses
    const SynapseMatrixType m_MatrixType;

    //! Pointer to presynaptic neuron group
    NeuronGroupInternal * const m_SrcNeuronGroup;

    //! Pointer to postsynaptic neuron group
    NeuronGroupInternal * const m_TrgNeuronGroup;

    //! Pointer to 'master' weight sharing group if this is a slave
    const SynapseGroupInternal *m_WeightSharingMaster;

    //! Does the event threshold needs to be retested in the synapse kernel?
    /*! This is required when the pre-synaptic neuron population's outgoing synapse groups require different event threshold */
    bool m_EventThresholdReTestRequired;

    //! Should narrow i.e. less than 32-bit types be used for sparse matrix indices
    bool m_NarrowSparseIndEnabled;

    //! Are any of this synapse group's weight update model variables referenced by a custom update
    bool m_WUVarReferencedByCustomUpdate;

    //! Variable mode used for variables used to combine input from this synapse group
    VarLocation m_InSynLocation;

    //! Variable mode used for this synapse group's dendritic delay buffers
    VarLocation m_DendriticDelayLocation;

    //! Weight update model type
    const WeightUpdateModels::Base *m_WUModel;

    //! Parameters of weight update model
    const std::vector<double> m_WUParams;

    //! Derived parameters for weight update model
    std::vector<double> m_WUDerivedParams;

    //! Initialisers for weight update model per-synapse variables
    std::vector<Models::VarInit> m_WUVarInitialisers;

    //! Initialisers for weight update model per-presynaptic neuron variables
    std::vector<Models::VarInit> m_WUPreVarInitialisers;

    //! Initialisers for weight update model post-presynaptic neuron variables
    std::vector<Models::VarInit> m_WUPostVarInitialisers;
    
    //! Post synapse update model type
    const PostsynapticModels::Base *m_PSModel;

    //! Parameters of post synapse model
    const std::vector<double> m_PSParams;

    //! Derived parameters for post synapse model
    std::vector<double> m_PSDerivedParams;

    //! Initialisers for post synapse model variables
    std::vector<Models::VarInit> m_PSVarInitialisers;

    //! Location of individual per-synapse state variables
    std::vector<VarLocation> m_WUVarLocation;

    //! Location of individual presynaptic state variables
    std::vector<VarLocation> m_WUPreVarLocation;

    //! Location of individual postsynaptic state variables
    std::vector<VarLocation> m_WUPostVarLocation;

    //! Location of weight update model extra global parameters
    std::vector<VarLocation> m_WUExtraGlobalParamLocation;

    //! Whether indidividual state variables of post synapse should use zero-copied memory
    std::vector<VarLocation> m_PSVarLocation;

    //! Location of postsynaptic model extra global parameters
    std::vector<VarLocation> m_PSExtraGlobalParamLocation;

    //! Initialiser used for creating sparse connectivity
    InitSparseConnectivitySnippet::Init m_ConnectivityInitialiser;

    //! Location of sparse connectivity
    VarLocation m_SparseConnectivityLocation;

    //! Location of connectivity initialiser extra global parameters
    std::vector<VarLocation> m_ConnectivityExtraGlobalParamLocation;

    //! Name of the synapse group in which postsynaptic model is located
    /*! This may not be the name of this group if it has been merged*/
    std::string m_PSModelTargetName;
};
