//-----------------------------------------------------------
// Description:
//   Analyzing pulse shape of raw signal and make it to a hit
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//   JungWoo Lee     Korea University
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

class STPSATask : public FairTask 
{
  public:
    STPSATask();
    ~STPSATask();

    enum STPSAMode
    {
      kSimple = 0,
      kAll    = 1,
      kLayer  = 2,
      kOPTICS = 3
    };

    void SetPSAMode(STPSAMode mode);
    void SetPersistence(Bool_t value = kTRUE);
    void SetThreshold(Double_t threshold);
    void SetLayerCut(Double_t layerCut);

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
    STPSAMode fPSAMode;

    Bool_t fIsPersistence;
    
    Double_t fThreshold;
    Int_t fLayerCut;

  ClassDef(STPSATask, 1);
};

#endif
