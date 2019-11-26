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

#ifndef STTRANSFORMFRAMETASK_HH
#define STTRANSFORMFRAMETASK_HH

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TH2.h"

// STL
#include <vector>

using std::vector;

class STTransformFrameTask : public FairTask {
  public:
    /// Constructor
    STTransformFrameTask();
    /// Destructor
    ~STTransformFrameTask();

    void SetBeamEnergyPerN(double energy);
    void SetBeamMass(int mass);
    
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);

  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fData;
    TClonesArray *fMass;
    TClonesArray *fPDG;
    TClonesArray *fCMVector;

    TVector3 fVBeam;
    Double_t fEnergy = 128;
    Int_t fMBeam = 124;
    const Double_t fNucleonMass = 931.5;
  
  ClassDef(STTransformFrameTask, 1);
};

#endif
