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

#ifndef _STPIDPROBTASK_H_
#define _STPIDPROBTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"
#include "TH2.h"
#include "TCutG.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TF1.h"
#include "TPaveText.h"

// STL
#include <vector>
#include <functional>
#include <string>

using std::vector;

class STPIDProbTask : public FairTask {
  public:
    /// Constructor
    STPIDProbTask();
    /// Destructor
    ~STPIDProbTask();

  
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value);

    void SetMetaFile(const std::string& t_metafile, bool t_update=false);
    void SetPIDFitFile(const std::string& t_fitfile);
  private:
    FairLogger *fLogger;                ///< FairLogger singleton

    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fData;
    std::map<int, STVectorF*> fPDGProb;

    bool fIterateMeta = false;
    TFile *fMetaFile = nullptr;
    TFile *fFitFile = nullptr;

    int fMinNClus = 15;
    double fMaxDPOCA = 20;
    std::map<int, TF1*> fBBE;            ///<modified Bethe Bloch equation for different particles 
    std::map<int, TF1*> fSigma;
    //std::function<double(double*,double*)> fBEETemp; ///<
    std::map<int, TH2F> fFlattenHist;
    std::map<int, TH1F> fMomPosteriorDistribution;
    std::map<int, TH1F*> fMomPriorDistribution;
    const std::vector<int> fSupportedPDG{2212, 1000010020, 1000010030, 1000020030, 1000020040};

    STVectorI *fPDGLists = nullptr;
    STVectorF *fMassLists = nullptr;
 
  ClassDef(STPIDProbTask, 1);
};

#endif
