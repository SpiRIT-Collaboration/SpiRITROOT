#include "STRecoTask.hh"

#include "FairRun.h"
#include "FairRuntimeDb.h"

ClassImp(STRecoTask);

STRecoTask::STRecoTask()
: FairTask()
{
  fIsPersistence = kFALSE;
}

STRecoTask::STRecoTask(const char* name, Int_t iVerbose, Bool_t persistence)
: FairTask(name, iVerbose)
{
  fIsPersistence = persistence;
}

void STRecoTask::SetPersistence(Bool_t value) { fIsPersistence = value; }

void STRecoTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    LOG(FATAL) << "FairRun Not Found!" << FairLogger::endl;

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    LOG(FATAL) << "FairRuntimeDb Not Found!" << FairLogger::endl;

  fDigiPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fDigiPar)
    LOG(FATAL) << "STDigiPar Not Found!" << FairLogger::endl;
}

InitStatus STRecoTask::Init()
{
  fRootManager = FairRootManager::Instance();
  if (fRootManager == nullptr) {
    LOG(ERROR) << "Cannot find RootManager!" << FairLogger::endl;
    return kERROR;
  }

  fRecoHeader = (STRecoHeader *) fRootManager -> GetOutFile() -> Get("RecoHeader");
  fEventHeader = (STEventHeader *) fRootManager -> GetObject("STEventHeader");

  return kSUCCESS;
}

TString STRecoTask::Space()
{
  Int_t length = 0;
  Int_t id = fEventHeader -> GetEventID();
  while (id /= 10)
    length++;

  TString space = "";
  for (Int_t i = 0; i < (length + 1 + 9); i++) space += " ";

  return space;
}
