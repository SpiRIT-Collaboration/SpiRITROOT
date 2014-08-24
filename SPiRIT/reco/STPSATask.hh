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

// ROOT classes
#include "TClonesArray.h" 

class STPSATask : public FairTask {
  public:
    STPSATask();
    ~STPSATask();

    void SetPersistence(Bool_t value = kTRUE);
    void SetThreshold(Double_t threshold);

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    FairLogger *fLogger;
    
    STDigiPar *fPar;
    TClonesArray *fRawEventArray;
    TClonesArray *fEventHArray;

    Bool_t fIsPersistence;
    
    Double_t fThreshold;
    Int_t fRunNo; /// Only for forcing run number

  ClassDef(STPSATask, 1);
};

#endif
