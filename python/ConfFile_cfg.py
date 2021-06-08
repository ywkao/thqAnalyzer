import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'file:/afs/cern.ch/work/y/ykao/public/forPrafulla/rootfiles/pickevents_1_132_131039_miniAOD.root'
        #'file:/afs/cern.ch/work/y/ykao/public/forPrafulla/rootfiles/pickevents_1_132_131124_miniAOD.root'
    )
)

#----------------------------------------------------------------------------------------------------
# Update JEC (from Stephanie)
# twiki page: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections
#----------------------------------------------------------------------------------------------------
from CondCore.CondDB.CondDB_cfi import *

process.jec = cms.ESSource('PoolDBESSource',
    connect = cms.string('sqlite:Summer19UL18_V5_MC.db'), #(--> To be adapted to the correction file you want to use)
    toGet = cms.VPSet(
        cms.PSet(
            record = cms.string('JetCorrectionsRecord'),
            tag    = cms.string('JetCorrectorParametersCollection_Summer19UL18_V5_MC_AK4PFchs'),
            label  = cms.untracked.string('AK4PFchs')
        ),
    )
)

# Add an ESPrefer to override JEC that might be available from the global tag
process.es_prefer_jec = cms.ESPrefer('PoolDBESSource', 'jec')

process.jeR = cms.ESSource('PoolDBESSource',
    connect = cms.string('sqlite:Summer19UL18_JRV2_MC.db'), #(--> To be adapted to the correction file you want to use)
    toGet = cms.VPSet(
        # Resolution
        cms.PSet(
            record = cms.string('JetResolutionRcd'),
            tag    = cms.string('JR_Summer19UL18_JRV2_MC_PtResolution_AK4PFchs'),
            label  = cms.untracked.string('AK4PFchs_pt')
        ),
        # Scale factors
        cms.PSet(
            record = cms.string('JetResolutionScaleFactorRcd'),
            tag    = cms.string('JR_Summer19UL18_JRV2_MC_SF_AK4PFchs'),
            label  = cms.untracked.string('AK4PFchs')
        ),
    )
)

# Add an ESPrefer to override JEC that might be available from the global tag
process.es_prefer_jeR = cms.ESPrefer('PoolDBESSource', 'jeR')

#/cvmfs/cms.cern.ch/slc7_amd64_gcc700/cms/cmssw/CMSSW_10_6_8/src/PhysicsTools/PatAlgos/python/tools/helpers.py
#/cvmfs/cms.cern.ch/slc7_amd64_gcc700/cms/cmssw/CMSSW_10_6_8/src/PhysicsTools/PatAlgos/python/tools/jetTools.py
from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection
updateJetCollection(
    process,
    jetSource = cms.InputTag('slimmedJets'),
    labelName = 'UpdatedJEC',
    jetCorrections = ('AK4PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute']), 'None'),
)

process.slimmedJetsSmeared = cms.EDProducer('SmearedPATJetProducer',
    #src = cms.InputTag('slimmedJets'),
    #src = cms.InputTag('updatedPatJetsUpdatedJEC'),
    src = cms.InputTag('selectedUpdatedPatJetsUpdatedJEC'),
    enabled = cms.bool(True),
    rho = cms.InputTag("fixedGridRhoFastjetAll"),
    algo = cms.string('AK4PFchs'),
    algopt = cms.string('AK4PFchs_pt'),

    genJets = cms.InputTag('slimmedGenJets'),
    dRMax = cms.double(0.2),
    dPtMaxFactor = cms.double(3),

    debug = cms.untracked.bool(False),
    # Systematic variation
    # 0: Nominal
    # -1: -1 sigma (down variation)
    # 1: +1 sigma (up variation)
    variation = cms.int32(0),  # If not specified, default to 0
)

#----------------------------------------------------------------------------------------------------

process.demo = cms.EDAnalyzer('thqAnalyzer',
    jetsColBeforeJER = cms.InputTag("selectedUpdatedPatJetsUpdatedJEC"),
    jetsCol = cms.InputTag('slimmedJetsSmeared'),
    jets = cms.InputTag("slimmedJets")
)

task = getattr(process, "patAlgosToolsTask")
process.p = cms.Path( process.slimmedJetsSmeared * process.demo, task )

process.out = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string('new_1_132_131039.root'),
    outputCommands = cms.untracked.vstring(
        "keep *"
    )
)

process.e = cms.EndPath(process.out)
