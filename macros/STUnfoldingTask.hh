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

#ifndef _STUNFOLDINGTASK_H_
#define _STUNFOLDINGTASK_H_

// FAIRROOT classes
//#include "FairTask.h"
//#include "FairLogger.h"
//
//// SPiRITROOT classes
//#include "STData.hh"
//#include "STDigiPar.hh"
//#include "STVector.hh"
//#include "EfficiencyFactory.hh"
//
//// ROOT classes
//#include "TClonesArray.h"
//#include "TLorentzVector.h"
//#include "TString.h"
//#include "TH2.h"
//#include "TGraph.h"
//#include "TUnfoldBinning.h"
//#include "TUnfoldDensity.h"
//
//// STL
//#include <vector>
//#include <memory>
//#include <string>

using std::vector;

class STUnfoldingTask : public FairTask {
  public:
    STUnfoldingTask();
    /// Destructor
    ~STUnfoldingTask();

    void SetMomBins(double t_min, double t_max, int t_det_bins, int t_gen_bins);
    void SetThetaBins(double t_min, double t_max, int t_det_bins, int t_gen_bins); 
    void LoadMCData(const std::string& t_treename = "cbmsim", 
                    const std::string& t_filename = "data/embed_dump/ImQMD_embedCorProton2/*");

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();

    void SetPersistence(Bool_t value);
    void SetCutConditions(int min_clusters, double max_dpoca);

  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    STVectorI *fPDG;                 ///<
    TClonesArray *fData;                ///< STData from the conc files

    TUnfoldBinning *fDetectorBinning = nullptr;
    TUnfoldBinning *fGeneratorBinning = nullptr;
    std::string fMCTreeName;
    std::string fMCFileName;

    Int_t fNDetTheta = 30;
    Int_t fNGenTheta = 15;
    Int_t fNDetMom = 30;
    Int_t fNGenMom = 15;
    Double_t fMinMom = 100;
    Double_t fMaxMom = 1300;
    Double_t fMinTheta = 0;
    Double_t fMaxTheta = 1.3;
   
    Int_t fMinClusters = 15;
    Double_t fMaxDPoca = 20;

    TH1*  fProtonHist = nullptr;
    TH2D *fResponseMatrix = nullptr;
  
  ClassDef(STUnfoldingTask, 1);
};

#endif
