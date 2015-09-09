//-----------------------------------------------------------
// Description:
//   Analyzing pulse shape of raw signal and make it to a hit
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//   JungWoo Lee     Korea University
//-----------------------------------------------------------

// SPiRITROOT classes
#include "STPSATask.hh"
#include "STPSA.hh"
#include "STPSASimple.hh"
#include "STPSAAll.hh"
#include "STPSALayer.hh"
#include "STPSALayerOPTICS.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(STPSATask);

STPSATask::STPSATask()
{
  fLogger = FairLogger::GetLogger();

  fIsPersistence = kFALSE;
  fEventHArray = new TClonesArray("STEvent");

  fPSAMode = kAll;
  fThreshold = 0;
  fLayerCut = 0;
}

STPSATask::~STPSATask()
{
}

void STPSATask::SetPSAMode(STPSAMode mode)       { fPSAMode = mode; }
void STPSATask::SetPersistence(Bool_t value)     { fIsPersistence = value; }
void STPSATask::SetThreshold(Double_t threshold) { fThreshold = threshold; }
void STPSATask::SetLayerCut(Double_t layerCut)   { fLayerCut = layerCut; }

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
  else {
    fLogger -> Fatal(MESSAGE_ORIGIN, "No Matching PSAMode!");
  }


  fPSA -> SetThreshold((Int_t)fThreshold);
  fPSA -> SetLayerCut(fLayerCut);

  if (fPSAMode == kOPTICS)
    ioMan -> Register("STEventHC", "SPiRIT", fEventHArray, fIsPersistence);
  else
    ioMan -> Register("STEventH", "SPiRIT", fEventHArray, fIsPersistence);

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
  fEventHArray -> Delete();

  if (fRawEventArray -> GetEntriesFast() == 0)
    return;

  STRawEvent *rawEvent = (STRawEvent *) fRawEventArray -> At(0);

  STEvent *event = (STEvent *) new ((*fEventHArray)[0]) STEvent();
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
}
