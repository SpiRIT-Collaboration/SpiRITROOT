#include "STAuxHeaderTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"

ClassImp(STAuxHeaderTask);

STAuxHeaderTask::STAuxHeaderTask()
{
  fLogger = FairLogger::GetLogger();
  fEventID = -1;
}

STAuxHeaderTask::~STAuxHeaderTask()
{
}

InitStatus STAuxHeaderTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");

    return kERROR;
  }

  fAuxHeader = new STAuxHeader();

  if(fInputAuxHeaderBranch != "")
   fInputAuxHeader = dynamic_cast<STAuxHeader *>(ioMan->GetObject(fInputAuxHeaderBranch));
  ioMan -> Register(fAuxHeaderBranch, "SpiRIT", fAuxHeader, fIsPersistence);

  fEventID = 0;

  return kSUCCESS;
}

void STAuxHeaderTask::Exec(Option_t *opt)
{
    if(fDoEventID) {
        if(fInputAuxHeader)
            SetEventNum(fInputAuxHeader -> GetTpcEventNum());
        else
            SetEventNum(fEventID++);
    }
}

void STAuxHeaderTask::SetEventNum(Int_t value)
{
    fAuxHeader -> SetTpcEventNum(value);
}

