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

#include "STPSATask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(STPSATask);

STPSATask::STPSATask()
{
  fPar = NULL;
}

STPSATask::~STPSATask()
{
}

InitStatus
STPSATask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    Error("STPSATask::Init()", "RootManager not instantiated!");
    return kERROR;
  }

  fRawEventArray = (TClonesArray *) ioMan -> GetObject("STRawEvent");
  if (fRawEventArray == 0) {
    Error("STPSATask::Init()", "Couldn't find STRawEvent array!");
    return kERROR;
  }

  return kSUCCESS;
}

void
STPSATask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    Fatal("STPSATask::SetParContainers()", "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    Fatal("STPSATask::SetParContainers()", "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    Fatal("STPSATask::SetParContainers()", "STDigiPar not found!!");

}

void
STPSATask::Exec(Option_t *opt)
{
  Int_t lastIdx = fRawEventArray -> GetEntriesFast() - 1;

  STRawEvent *test = (STRawEvent *) fRawEventArray -> At(lastIdx);
  std::cout << test -> GetEventID() << " " << test -> GetNumPads() << std::endl;  
}
