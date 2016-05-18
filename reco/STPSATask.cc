/**
 * @brief Analyzing pulse shape of raw signal and make it to a hit
 *
 * Author List:
 * @author Genie Jhang (Korea University), original author
 * @author JungWoo Lee (Korea University)
 */

// SPiRITROOT classes
#include "STPSATask.hh"
#include "STPSA.hh"
#include "STPSASimple.hh"
#include "STPSAAll.hh"
#include "STPSALayer.hh"
#include "STPSALayerOPTICS.hh"
#include "STPSADF.hh"
#include "STPSAFast.hh"
#include "STPSAFastFit.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(STPSATask);

STPSATask::STPSATask()
{
  fIsPersistence = kFALSE;

  fLogger = FairLogger::GetLogger();

  fEventArray = new TClonesArray("STEvent");

  fPSAMode = kFastFit;
  fThreshold = 0;
  fLayerLowCut = -1;
  fLayerHighCut = 112;

  fPulserDataName = "";
  fShapingTime = 117;
}

STPSATask::STPSATask(Bool_t persistence, Double_t threshold)
:STPSATask()
{
  fIsPersistence = persistence;
  fThreshold = threshold;
}

STPSATask::~STPSATask()
{
}

void STPSATask::SetPersistence(Bool_t value)     { fIsPersistence = value; }
void STPSATask::SetPSAMode(STPSAMode mode)       { fPSAMode = mode; }
void STPSATask::SetThreshold(Double_t threshold) { fThreshold = threshold; }
void STPSATask::SetLayerCut(Int_t lowCut, Int_t highCut)
{
  fLayerLowCut = lowCut;
  fLayerHighCut = highCut;
}

void STPSATask::SetPulserData(TString pulserData) { fPulserDataName = pulserData; }
void STPSATask::UseDefautPulserData(Int_t shapingTime) { fShapingTime = shapingTime; }

STPSA* STPSATask::GetPSA() { return fPSA; }

InitStatus
STPSATask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fRawEventArray = (TClonesArray *) ioMan -> GetObject("STRawEvent");
  if (fRawEventArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STRawEvent array!");
    return kERROR;
  }

  if (fPSAMode == kSimple) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSASimple!");
    fPSA = new STPSASimple();
  } 
  else if (fPSAMode == kAll) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSAAll!");
    fPSA = new STPSAAll();
  } 
  else if (fPSAMode == kLayer) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSALayer!");
    fPSA = new STPSALayer();
  } 
  else if (fPSAMode == kOPTICS) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSALayerOPTICS!");
    fPSA = new STPSALayerOPTICS();
  }
  else if (fPSAMode == kDF) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSADF!");
    fPSA = new STPSADF();
  }
  else if (fPSAMode == kFast) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSAFast!");
    fPSA = new STPSAFast();
  }
  else if (fPSAMode == kFastFit) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSAFastFit!");
    if (!fPulserDataName.IsNull())
      fPSA = new STPSAFastFit(fPulserDataName);
    else
      fPSA = new STPSAFastFit(fShapingTime);
  }
  else {
    fLogger -> Fatal(MESSAGE_ORIGIN, "No Matching PSAMode!");
  }


  fPSA -> SetThreshold((Int_t)fThreshold);
  fPSA -> SetLayerCut(fLayerLowCut, fLayerHighCut);

  ioMan -> Register("STEvent", "SPiRIT", fEventArray, fIsPersistence);

  return kSUCCESS;
}

void
STPSATask::SetParContainers()
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
STPSATask::Exec(Option_t *opt)
{
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStart("PSATask");
#endif
  fEventArray -> Delete();

  if (fRawEventArray -> GetEntriesFast() == 0)
    return;

  STRawEvent *rawEvent = (STRawEvent *) fRawEventArray -> At(0);

  STEvent *event = (STEvent *) new ((*fEventArray)[0]) STEvent();
  event -> SetEventID(rawEvent -> GetEventID());

  if (!(rawEvent -> IsGood())) {
    event -> SetIsGood(kFALSE);
    fLogger -> Info(MESSAGE_ORIGIN, Form("Event #%d : Bad event!", rawEvent -> GetEventID()));
  }
  else {
    fPSA -> Analyze(rawEvent, event);
    if (event -> GetNumHits() == 0) {
      event -> SetIsGood(kFALSE);
      fLogger -> Info(MESSAGE_ORIGIN, Form("Event #%d : No hits found!", rawEvent -> GetEventID()));
    }
    else {
      event -> SetIsGood(kTRUE);
      fLogger -> Info(MESSAGE_ORIGIN, 
                    Form("Event #%d : Reconstructed %d hits and %d clusters.",
                         rawEvent -> GetEventID(), event -> GetNumHits(), event -> GetNumClusters()));
    }
  }
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("PSATask");
#endif
}
