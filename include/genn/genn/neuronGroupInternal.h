#pragma once

// GeNN includes
#include "neuronGroup.h"

//------------------------------------------------------------------------
// GeNN::NeuronGroupInternal
//------------------------------------------------------------------------
namespace GeNN
{
class NeuronGroupInternal : public NeuronGroup
{
public:
    NeuronGroupInternal(const std::string &name, int numNeurons, const NeuronModels::Base *neuronModel,
                        const std::unordered_map<std::string, double> &params, const std::unordered_map<std::string, Models::VarInit> &varInitialisers,
                        VarLocation defaultVarLocation, VarLocation defaultExtraGlobalParamLocation)
    :   NeuronGroup(name, numNeurons, neuronModel, params, varInitialisers,
                    defaultVarLocation, defaultExtraGlobalParamLocation)
    {
    }
    
    using NeuronGroup::checkNumDelaySlots;
    using NeuronGroup::updatePreVarQueues;
    using NeuronGroup::updatePostVarQueues;
    using NeuronGroup::addSpkEventCondition;
    using NeuronGroup::addInSyn;
    using NeuronGroup::addOutSyn;
    using NeuronGroup::finalise;
    using NeuronGroup::fusePrePostSynapses;
    using NeuronGroup::injectCurrent;
    using NeuronGroup::getFusedPSMInSyn;
    using NeuronGroup::getFusedWUPostInSyn;
    using NeuronGroup::getFusedPreOutputOutSyn;
    using NeuronGroup::getFusedWUPreOutSyn;
    using NeuronGroup::getOutSyn;
    using NeuronGroup::getCurrentSources;
    using NeuronGroup::getDerivedParams;
    using NeuronGroup::getSpikeEventCondition;
    using NeuronGroup::getFusedInSynWithPostCode;
    using NeuronGroup::getFusedOutSynWithPreCode;
    using NeuronGroup::getFusedInSynWithPostVars;
    using NeuronGroup::getFusedOutSynWithPreVars;
    using NeuronGroup::isSimRNGRequired;
    using NeuronGroup::isInitRNGRequired;
    using NeuronGroup::isVarQueueRequired;
    using NeuronGroup::getHashDigest;
    using NeuronGroup::getInitHashDigest;
    using NeuronGroup::getSpikeQueueUpdateHashDigest;
    using NeuronGroup::getPrevSpikeTimeUpdateHashDigest;
    using NeuronGroup::getVarLocationHashDigest;
};

//----------------------------------------------------------------------------
// NeuronVarAdapter
//----------------------------------------------------------------------------
class NeuronVarAdapter
{
public:
    NeuronVarAdapter(const NeuronGroupInternal &ng) : m_NG(ng)
    {}

    //----------------------------------------------------------------------------
    // Public methods
    //----------------------------------------------------------------------------
    VarLocation getLoc(const std::string &varName) const{ return m_NG.getVarLocation(varName); }
    
    Models::Base::VarVec getDefs() const{ return m_NG.getNeuronModel()->getVars(); }

    const std::unordered_map<std::string, Models::VarInit> &getInitialisers() const{ return m_NG.getVarInitialisers(); }

    bool isVarDelayed(const std::string &varName) const{ return m_NG.isVarQueueRequired(varName); }

    const std::string &getNameSuffix() const{ return m_NG.getName(); }

private:
    //----------------------------------------------------------------------------
    // Members
    //----------------------------------------------------------------------------
    const NeuronGroupInternal &m_NG;
};

//----------------------------------------------------------------------------
// NeuronEGPAdapter
//----------------------------------------------------------------------------
class NeuronEGPAdapter
{
public:
    NeuronEGPAdapter(const NeuronGroupInternal &ng) : m_NG(ng)
    {}

    //----------------------------------------------------------------------------
    // Public methods
    //----------------------------------------------------------------------------
    VarLocation getLoc(const std::string &varName) const{ return m_NG.getExtraGlobalParamLocation(varName); }

    Snippet::Base::EGPVec getDefs() const{ return m_NG.getNeuronModel()->getExtraGlobalParams(); }

private:
    //----------------------------------------------------------------------------
    // Members
    //----------------------------------------------------------------------------
    const NeuronGroupInternal &m_NG;
};
}   // namespace GeNN
