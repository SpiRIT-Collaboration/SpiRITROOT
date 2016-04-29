/**
 * @author JungWoo Lee
 *
 * @brief STCurveTrackingTask
 */

// SpiRITROOT classes
#include "STEvent.hh"
#include "STCurveTrackingTask.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"

// FairROOT classes
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"

#include <iostream>

using namespace std;

ClassImp(STCurveTrackingTask);

STCurveTrackingTask::STCurveTrackingTask()
:FairTask("ST Curve Tracking")
{
  fIsPersistence = kFALSE;

  fLogger = FairLogger::GetLogger();
}

STCurveTrackingTask::STCurveTrackingTask(Bool_t persistence)
:STCurveTrackingTask()
{
  fIsPersistence = persistence;
}

void STCurveTrackingTask::SetPersistence(Bool_t value) { fIsPersistence = value; }

STCurveTrackingTask::~STCurveTrackingTask()
{
}

InitStatus
STCurveTrackingTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance(); 
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find FairRootManager!");
    return kERROR;
  }
  
  fEventArray = (TClonesArray *) ioMan -> GetObject("STEvent");
  if (fEventArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEvent array!");
    return kERROR;
  }
  
  fTrackArray = new TClonesArray("STCurveTrack", 20);

  ioMan -> Register("STCurveTrack", "SPiRIT", fTrackArray, fIsPersistence);

  fTrackFinder = new STCurveTrackFinder();

  return kSUCCESS;
}


void
STCurveTrackingTask::SetParContainers()
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
STCurveTrackingTask::Exec(Option_t *opt)
{
#ifdef SUBTASK_CURVETRACKING
  if (opt == TString("sub")) {
    fLogger -> Info(MESSAGE_ORIGIN, "Running sub task");
    fTrackFinder -> AnaNextHit();
  }
  else {
    fTrackArray -> Delete();

    STEvent *event = (STEvent *) fEventArray -> At(0);
    fTrackFinder -> Init(event, fTrackArray);
  }
#endif

#ifndef SUBTASK_CURVETRACKING
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStart("CurveTrackingTask");
#endif

  fTrackArray -> Delete();

  STEvent *event = (STEvent *) fEventArray -> At(0);

  if (event -> IsGood() == kFALSE)
  {
    fLogger -> Info(MESSAGE_ORIGIN, Form("Event #%d : Bad event!", event -> GetEventID()));
    return;
  }

  fTrackFinder -> BuildTracks(event, fTrackArray);

  fLogger -> Info(MESSAGE_ORIGIN, 
    Form("Event #%d : Found %d curve tracks", 
      event -> GetEventID(), fTrackArray -> GetEntriesFast()));

#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("CurveTrackingTask");
#endif
#endif
}
