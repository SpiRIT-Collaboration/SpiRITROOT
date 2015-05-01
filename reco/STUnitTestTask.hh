//---------------------------------------------------------------------
// Description:
//      UnitTest task class.
//      This is for testing simple task.
//
// Author List:
//      Genie Jhang     Korea Univ.       (original author)
//----------------------------------------------------------------------

#ifndef STUNITTESTTASK_H
#define STUNITTESTTASK_H

// SpiRITROOT classes
#include "STDigiPar.hh"

// FairRoot classes
#include "FairLogger.h"
#include "FairTask.h"

// ROOT classes
#include "TClonesArray.h"

class STUnitTestTask : public FairTask
{
  public:
    STUnitTestTask();
    ~STUnitTestTask();
    
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    FairLogger *fLogger;
    STDigiPar *fPar;

    TClonesArray *fEventHCArray;
    TClonesArray *fEventHCMArray;

  ClassDef(STUnitTestTask, 1);
};

#endif
