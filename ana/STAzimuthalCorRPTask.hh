#ifndef _STAZIMUTHALCORROTASK_H_
#define _STAZIMUTHALCORROTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "EfficiencyFactory.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"

// STL
#include <vector>
#include <memory>
#include <string>

using std::vector;

class STAzimuthalCorRPTask : public FairTask {
  public:
    STAzimuthalCorRPTask();
    /// Destructor
    ~STAzimuthalCorRPTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value);
    void SetCutConditions(int min_clusters, double max_dpoca);

  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fData = nullptr;                ///< STData from the conc files
    TClonesArray *fProb = nullptr;                 ///<
    TClonesArray *fCMVector = nullptr;
    TClonesArray *fFragRapidity = nullptr;
    STVectorF *fBeamRapidity = nullptr;

    TClonesArray *fPhiEff = nullptr;
    STVectorF *fReactionPlane = nullptr;
    STVectorF *fReactionPlaneV2 = nullptr;
    TClonesArray *fV1RPAngle = nullptr;

    Int_t fMinNClusters = 5;
    Double_t fMaxDPOCA = 20;
    std::vector<int> fSupportedPDG;

  ClassDef(STAzimuthalCorRPTask, 1);
};

#endif
