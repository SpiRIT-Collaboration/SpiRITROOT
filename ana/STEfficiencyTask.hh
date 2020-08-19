//-----------------------------------------------------------
// Description:
//   Embed pulses onto the data
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//   Tommy Tsang     MSU                  (decouple this class from STDecoder class)
//-----------------------------------------------------------

#ifndef _STEFFICIENCYTASK_H_
#define _STEFFICIENCYTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "EfficiencyFactory.hh"
#include "STAnaParticleDB.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TH2.h"

// STL
#include <vector>
#include <memory>

using std::vector;

class STEfficiencyTask : public FairTask {
  public:
    struct EfficiencySettings
    {int NClusters = 15; double DPoca = 20; 
     int NThetaBins = 20; double ThetaMin = 0; double ThetaMax = 90;
     int NMomBins = 20; double MomMin = 0; double MomMax = 3000;
     int NCMzBins = 20; double CMzMin = -1000; double CMzMax = 1000;
     int NPtBins = 20; double PtMin = 0; double PtMax = 1500; 
     std::vector<std::pair<double, double>> PhiCuts = {{0,360}};};

    /// Constructor
    STEfficiencyTask();
    /// Destructor
    ~STEfficiencyTask();

    void SetFactoriesForParticle(EfficiencyFactory* t_factory, const std::vector<int>& pdgList);
    void SetUnfoldingFile(TString fileName, bool update = false) { fUnfoldingFileName = fileName; fUpdateUnfolding = update; }
    void UpdateUnfoldingFile(bool update) { fUpdateUnfolding = update; }
    EfficiencySettings& AccessSettings(int t_pdg) { return fEfficiencySettings[t_pdg]; }
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value);

  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar      = nullptr;                 ///< Parameter read-out class pointer
    STVectorI *fPDG      = nullptr;                 ///<
    TClonesArray *fData  = nullptr;                 ///< STData from the conc files
    TClonesArray *fCMVec = nullptr;                 ///< vector in CM frame
    TClonesArray *fEff   = nullptr;                 ///< Efficiency of each type of particle
    TClonesArray *fEffErr = nullptr;                ///< Average efficiency error (0.5*(up err + down err))
    TClonesArray *fProb  = nullptr;                 ///< Particle PID from any of the PID Task
    STVectorI *fSkip = nullptr;

    const std::vector<int> fSupportedPDG = STAnaParticleDB::SupportedPDG;
    std::map<int, EfficiencyFactory*> fFactory; ///<
    std::map<int, TEfficiency> fEfficiency; ///<
    std::map<int, EfficiencySettings> fEfficiencySettings; ///<
    std::map<int, TH2F> fDistributionForUnfolding; //<
    TString fUnfoldingFileName;
    std::unique_ptr<TFile> fUnfoldingFile;
    bool fUpdateUnfolding = false;
  
  ClassDef(STEfficiencyTask, 1);
};

#endif
