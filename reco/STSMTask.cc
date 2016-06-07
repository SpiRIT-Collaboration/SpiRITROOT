//-----------------------------------------------------------
// Description:
//   System coordinate manipulating task
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

// SPiRITROOT classes
#include "STSMTask.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"
#include "STParReader.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(STSMTask);

STSMTask::STSMTask()
{
  fIsPersistence = kFALSE;

  fLogger = FairLogger::GetLogger();
  fPar = NULL;

  fEventHCMArray = new TClonesArray("STEvent");

  fManipulator = new STSystemManipulator();
  fSMMode = kChange;

  fVertexFlag = kFALSE;
}

STSMTask::STSMTask(Bool_t persistence, ESMMode mode)
:STSMTask()
{
  fIsPersistence = persistence;
  fSMMode = mode;
}

STSMTask::~STSMTask()
{
}

void STSMTask::SetPersistence(Bool_t value)      { fIsPersistence = value; }
void STSMTask::SetMode(ESMMode mode)             { fSMMode = mode; }
void STSMTask::SetTrans(TVector3 trans)          { fManipulator -> SetTrans(trans); }
void STSMTask::UseVertexFromParFile(Bool_t flag) { fVertexFlag = flag; } 

InitStatus
STSMTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fEventHCArray = (TClonesArray *) ioMan -> GetObject("STEvent");
  if (fEventHCArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEvent array!");

    return kERROR;
  }

  ioMan -> Register("STEventHCM", "SPiRIT", fEventHCMArray, fIsPersistence);

  if (fVertexFlag)
  {
    TString parName = fPar -> GetTrackingParFileName();
    STParReader *tpar = new STParReader(parName);

    fManipulator -> SetTrans(TVector3(tpar -> GetDoublePar("XVertex"),
          tpar -> GetDoublePar("YVertex"),
          tpar -> GetDoublePar("ZVertex")));
  }

  return kSUCCESS;
}

void
STSMTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");
}

void
STSMTask::Exec(Option_t *opt)
{
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStart("SMTask");
#endif
  fEventHCMArray -> Delete();

  if (fEventHCArray -> GetEntriesFast() == 0)
    return;

  STEvent *eventHC = (STEvent *) fEventHCArray -> At(0);

  STEvent *eventHCM = NULL;
  if (fSMMode == kChange)
  {
    eventHCM = fManipulator -> Change(eventHC);
    fLogger->Info(MESSAGE_ORIGIN, 
                  Form("Event #%d : Found %d clusters and manipulated coordinates.",
                       eventHCM -> GetEventID(), eventHCM -> GetNumClusters()));
  }
  else if (fSMMode == kRestore)
  {
    eventHCM = fManipulator -> Restore(eventHC);
    fLogger->Info(MESSAGE_ORIGIN, 
                  Form("Event #%d : Found %d clusters and restored coordinates.",
                       eventHCM -> GetEventID(), eventHCM -> GetNumClusters()));
  }

  new ((*fEventHCMArray)[0]) STEvent(eventHCM);
  delete eventHCM;
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("SMTask");
#endif
}
