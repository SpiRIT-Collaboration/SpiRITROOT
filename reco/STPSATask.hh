//-----------------------------------------------------------
// Description:
//   Analyzing pulse shape of raw signal and make it to a hit
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef _STPSATASK_H_
#define _STPSATASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STRawEvent.hh"
#include "STDigiPar.hh"
#include "STPSA.hh"

// ROOT classes
#include "TClonesArray.h" 

class STPSATask : public FairTask {
  public:
    STPSATask();
    ~STPSATask();

    void SetPSAMode(Int_t value = 0);
    void SetPersistence(Bool_t value = kTRUE);
    void SetThreshold(Double_t threshold);

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    STPSA* GetPSA();

  private:
    FairLogger *fLogger;
    
    STDigiPar *fPar;
    TClonesArray *fRawEventArray;
    TClonesArray *fEventHArray;

    STPSA *fPSA;
    Int_t fPSAMode;

    Bool_t fIsPersistence;
    
    Double_t fThreshold;

  ClassDef(STPSATask, 1);
};

#endif
