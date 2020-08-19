#ifndef _STPIPROBTASK_H_
#define _STPIPROBTASK_H_

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
#include "TF1.h"

// STL
#include <vector>
#include <memory>

class STPiProbTask : public FairTask {
  public:
    /// Constructor
    STPiProbTask();
    /// Destructor
    ~STPiProbTask();

    void ReadFile(TString filename);

    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);

  private:
    int SetParameters(double mom, double dedx);
 
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar      = nullptr;                 ///< Parameter read-out class pointer
    TClonesArray *fData  = nullptr;
    TClonesArray *fProb  = nullptr;
    TClonesArray *fEff   = nullptr;
    STVectorI *fSkip     = nullptr;
    TClonesArray *fFlattenPID = nullptr;

    double pitchBinSize = 20;
    double yawBinSize = 180;
    int numPitch = 200/pitchBinSize;
    int numPhi = 360/yawBinSize;
    int numHists = numPitch*numPhi;

    double momRange[300][2] = {{0}};
    double gausParam[300][9] = {{0}};
    double pidFitParam[36][5] = {{0}};
    int numSlices = 0;

    TF1 totalFunc, piFunc, bgFunc;
    int fPiPlusID, fPiMinusID;
    TF1 fitFunc;
  
  ClassDef(STPiProbTask, 1);
};

#endif
