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

#ifndef STSMTASK
#define STSMTASK


// SpiRITROOT classes
#include "STSystemManipulator.hh"
#include "STDigiPar.hh"

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// ROOT classes
#include "TClonesArray.h" 

class STSMTask : public FairTask {
  public:
    STSMTask();
    ~STSMTask();

    enum ESMMode { kChange, kRestore };

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    void SetMode(STSMTask::ESMMode mode);
    void SetPersistence(Bool_t value = kTRUE);

  private:
    FairLogger *fLogger;
    
    STDigiPar *fPar;
    TClonesArray *fEventHCArray;
    TClonesArray *fEventHCMArray;

    STSystemManipulator *fManipulator;

    ESMMode fSMMode;
    Bool_t fIsPersistence;

  ClassDef(STSMTask, 1);
};

#endif
