#ifndef STRECOTASK_HH
#define STRECOTASK_HH

#include "FairTask.h"
#include "FairLogger.h"
#include "FairRootManager.h"

#include "STDebugLogger.hh"
#include "STGlobal.hh"
#include "STDigiPar.hh"
#include "STRecoHeader.hh"
#include "STEventHeader.hh"

#include "TClonesArray.h" 

class STRecoTask : public FairTask 
{
  public:
    STRecoTask();
    STRecoTask(const char* name, Int_t iVerbose = 1, Bool_t persistence = false);
    virtual ~STRecoTask() {};

    virtual InitStatus Init();
    virtual void SetPersistence(Bool_t value = kTRUE);

    virtual void SetParContainers();

  protected:
    Bool_t fIsPersistence;  ///< Persistence check variable

    STRecoHeader *fRecoHeader = nullptr;
    STEventHeader *fEventHeader = nullptr;

    FairRootManager *fRootManager;

    STDigiPar *fDigiPar;

    TString Space();

  ClassDef(STRecoTask, 1);
};

#endif
