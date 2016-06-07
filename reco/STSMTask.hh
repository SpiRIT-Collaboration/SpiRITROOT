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
    enum ESMMode { kChange, kRestore };

    STSMTask();
    STSMTask(Bool_t persistence, ESMMode mode = STSMTask::kChange);
    ~STSMTask();

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value = kTRUE);

    void SetMode(STSMTask::ESMMode mode);

    void SetTrans(TVector3 trans);
    void UseVertexFromParFile(Bool_t flag = kTRUE);

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    FairLogger *fLogger;  //!
    
    STDigiPar *fPar;
    TClonesArray *fEventHCArray;
    TClonesArray *fEventHCMArray;

    STSystemManipulator *fManipulator;

    ESMMode fSMMode;

    Bool_t fVertexFlag;

  ClassDef(STSMTask, 1);
};

#endif
