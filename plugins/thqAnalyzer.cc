// -*- C++ -*-
//
// Package:    miniAOD_pickevents/thqAnalyzer
// Class:      thqAnalyzer
//
/**\class thqAnalyzer thqAnalyzer.cc miniAOD_pickevents/thqAnalyzer/plugins/thqAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Yu-Wei Kao
//         Created:  Thu, 03 Jun 2021 03:36:43 GMT
//
//


// system include files
#include <memory>
#include <string>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/Common/interface/Handle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

#include "JetMETCorrections/Modules/interface/JetResolution.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.


//using reco::TrackCollection;

class thqAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit thqAnalyzer(const edm::ParameterSet&);
      ~thqAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
      edm::EDGetTokenT<std::vector<pat::Jet>> jetToken_;
      edm::EDGetTokenT<std::vector<pat::Jet>> jetsColBeforeJERToken_;
      edm::EDGetTokenT<std::vector<pat::Jet>> jetsColToken_;
      edm::EDGetTokenT<double> m_rho_token;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
thqAnalyzer::thqAnalyzer(const edm::ParameterSet& iConfig):
    jetToken_(consumes<std::vector<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jets"))),
    jetsColBeforeJERToken_(consumes<std::vector<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jetsColBeforeJER"))),
    jetsColToken_(consumes<std::vector<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jetsCol"))),
    m_rho_token( consumes<double>(iConfig.getParameter<edm::InputTag>("rho")) )
{
   //now do what ever initialization is needed

}


thqAnalyzer::~thqAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
thqAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;

    JME::JetResolution resolution = JME::JetResolution::get(iSetup, "AK4PFchs_pt");
    JME::JetResolutionScaleFactor resolution_sf = JME::JetResolutionScaleFactor::get(iSetup, "AK4PFchs");

    edm::Handle<double> rho;
    iEvent.getByToken(m_rho_token, rho);

    printf("\n--------------------------------------------------\n\n");
    printf(">>> slimmedJets \n");
    edm::Handle<std::vector<pat::Jet>> jets;
    iEvent.getByToken(jetToken_, jets);
    int ijet = 0;
    for (const pat::Jet &j : *jets) {
        ijet += 1;
        std::cout << "Jet " << ijet << ": Pt " << j.pt() << " eta " << j.eta() << " phi " << j.phi() << " E " << j.energy() << std::endl;

        double jet_resolution = resolution.getResolution(
            {{JME::Binning::JetPt, j.pt()}, {JME::Binning::JetEta, j.eta()}, {JME::Binning::Rho, *rho}});
        double jer_sf = resolution_sf.getScaleFactor({{JME::Binning::JetPt, j.pt()}, {JME::Binning::JetEta, j.eta()}},
                                                     Variation::NOMINAL,
                                                     "");

        std::cout << "jet_resolution: " << jet_resolution << ", jer_sf: " << jer_sf << std::endl;
    }


    printf("\n--------------------------------------------------\n\n");
    printf(">>> selectedUpdatedPatJetsUpdatedJEC \n");
    edm::Handle<std::vector<pat::Jet>> jetsColBeforeJER;
    iEvent.getByToken(jetsColBeforeJERToken_, jetsColBeforeJER);
    ijet = 0;
    for (const pat::Jet &j : *jetsColBeforeJER) {
        ijet += 1;
        std::cout << "Jet " << ijet << ": Pt " << j.pt() << " eta " << j.eta() << " phi " << j.phi() << " E " << j.energy() << std::endl;
    }


    printf("\n--------------------------------------------------\n\n");
    printf(">>> slimmedJetsSmeared \n");
    edm::Handle<std::vector<pat::Jet>> jetsCol;
    iEvent.getByToken(jetsColToken_, jetsCol);
    ijet = 0;
    for (const pat::Jet &j : *jetsCol) {
        ijet += 1;
        std::cout << "Jet " << ijet << ": Pt " << j.pt() << " eta " << j.eta() << " phi " << j.phi() << " E " << j.energy() << std::endl;
    }


    printf("\n--------------------------------------------------\n\n");

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
#endif
}


// ------------ method called once each job just before starting event loop  ------------
void
thqAnalyzer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void
thqAnalyzer::endJob()
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
thqAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(thqAnalyzer);
