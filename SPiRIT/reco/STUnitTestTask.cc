//---------------------------------------------------------------------
// Description:
//      UnitTest task class.
//      This is for testing simple task.
//
// Author List:
//      Genie Jhang     Korea Univ.       (original author)
//----------------------------------------------------------------------

// SpiRITROOT classes
#include "STUnitTestTask.hh"
#include "STEvent.hh"
#include "STSystemManipulator.hh"

// FairRoot classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

ClassImp(STUnitTestTask);

STUnitTestTask::STUnitTestTask()
{
  fLogger = FairLogger::GetLogger();
}

STUnitTestTask::~STUnitTestTask()
{
}

InitStatus
STUnitTestTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");

    return kERROR;
  }

  fEventHCArray = (TClonesArray *) ioMan -> GetObject("STEventHC");
  if (fEventHCArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEventHC!");

    return kERROR;
  }

  fEventHCMArray = new TClonesArray("STEvent");
  ioMan -> Register("STEventHCM", "SPiRIT", fEventHCMArray, kTRUE);

  return kSUCCESS;
}

void
STUnitTestTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void
STUnitTestTask::Exec(Option_t *opt)
{
  fEventHCMArray -> Delete();

  STEvent *eventHC = (STEvent *) fEventHCArray -> At(0);

  STSystemManipulator manipulator = STSystemManipulator();
  new ((*fEventHCMArray)[0]) STEvent(manipulator.Manipulate(eventHC));
  STEvent *eventHCM = (STEvent *) fEventHCMArray -> At(0);
}
