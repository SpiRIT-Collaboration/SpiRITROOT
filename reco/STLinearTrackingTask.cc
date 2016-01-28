/**
 * @author JungWoo Lee
 */

// SpiRITROOT classes
#include "STEvent.hh"
#include "STLinearTrackingTask.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"

// FairROOT classes
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"

#include <iostream>

using namespace std;

ClassImp(STLinearTrackingTask);

STLinearTrackingTask::STLinearTrackingTask()
:FairTask("ST Linear Tracking")
{
  fIsPersistence = kFALSE;

  fLogger = FairLogger::GetLogger();
}

void STLinearTrackingTask::SetPersistence(Bool_t value) { fIsPersistence = value; }

STLinearTrackingTask::~STLinearTrackingTask()
{
}

InitStatus
STLinearTrackingTask::Init()
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
  
  fTrackArray = new TClonesArray("STLinearTrack");

  ioMan -> Register("STLinearTrack", "SPiRIT", fTrackArray, fIsPersistence);

  fTrackBuffer = new std::vector<STLinearTrack*>;

  fTrackFinder = new STLinearTrackFinder();

  return kSUCCESS;
}


void
STLinearTrackingTask::SetParContainers()
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
STLinearTrackingTask::Exec(Option_t *opt)
{
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStart("LinearTrackingTask");
#endif
  if(!fTrackArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No TrackArray!");
  fTrackArray -> Delete();

  fTrackBuffer -> clear();

  STEvent *event = (STEvent *) fEventArray -> At(0);

  fTrackFinder -> BuildTracks(event, fTrackBuffer);

  Int_t nTracks = fTrackBuffer -> size();
  for (Int_t iTrack = 0; iTrack < nTracks; iTrack++)
  {
    STLinearTrack* trackCand = fTrackBuffer -> at(iTrack);
    Int_t index = fTrackArray -> GetEntriesFast();
    STLinearTrack *track = new ((*fTrackArray)[index]) STLinearTrack(trackCand);
  }

  fLogger -> Info(MESSAGE_ORIGIN, 
    Form("Event #%d : Found %d linear tracks", 
      event -> GetEventID(), fTrackArray -> GetEntriesFast()));
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("LinearTrackingTask");
#endif
}

STVTrackFinder* STLinearTrackingTask::GetTrackFinder() { return fTrackFinder; }
