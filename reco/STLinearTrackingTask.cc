/**
 * @author JungWoo Lee
 */

// SpiRITROOT classes
#include "STEvent.hh"
#include "STLinearTrackingTask.hh"

// FairROOT classes
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"

// ROOT classes
#include "TMath.h"

ClassImp(STLinearTrackingTask);

STLinearTrackingTask::STLinearTrackingTask()
:FairTask("ST Pattern Reconstruction")
{
  fLogger = FairLogger::GetLogger();
}

STLinearTrackingTask::~STLinearTrackingTask()
{
}

InitStatus
STLinearTrackingTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance(); 
  if (ioMan == 0){
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find FairRootManager!");
    return kERROR;
  }
  
  fEventArray = (TClonesArray *) ioMan -> GetObject("STEventHCM");
  if (fEventArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEventHCM array!");
    return kERROR;
  }

  fTrackArray = new TClonesArray("STRiemannTrack");
  ioMan -> Register("STRiemannTrack", "SPiRIT", fTrackArray, fInputPersistance);

  return kSUCCESS;
}


void
STLinearTrackingTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run) fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db) fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar) fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}


void
STLinearTrackingTask::Exec(Option_t *opt)
{
  if (fTrackArray == 0)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find RiemannTrackArray!");
  fTrackArray -> Delete();

  if (fEventArray -> GetEntriesFast() == 0)
    return;
}
