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

// SPiRITROOT classes
#include "STPSATask.hh"
#include "STPSA.hh"
#include "STPSASimple.hh"
#include "STPSASimple2.hh"
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
  fPar = NULL;

  fIsPersistence = kFALSE;
  
  fEventHArray = new TClonesArray("STEvent");

  fPSAMode = 0;
}

STPSATask::~STPSATask()
{
}

void STPSATask::SetPSAMode(Int_t value)          { fPSAMode = value; }
void STPSATask::SetPersistence(Bool_t value)     { fIsPersistence = value; }
void STPSATask::SetThreshold(Double_t threshold) { fThreshold = threshold; }

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

  if (fPSAMode == 0) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSASimple!");

    fPSA = new STPSASimple();
  } else if (fPSAMode == 1) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSASimple2!");

    fPSA = new STPSASimple2();
  } else if (fPSAMode == 2) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSALayer!");

    fPSA = new STPSALayer();
  } else if (fPSAMode == 3) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STPSALayerOPTICS!");

    fPSA = new STPSALayerOPTICS();
  }


  fPSA -> SetThreshold((Int_t)fThreshold);

  if (fPSAMode == 3)
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
    fLogger->Info(MESSAGE_ORIGIN, Form("Event #%d : Bad event!", rawEvent -> GetEventID()));
  }
  else {
    fPSA -> Analyze(rawEvent, event);
    if (event -> GetNumHits() == 0) {
      event -> SetIsGood(kFALSE);
      fLogger->Info(MESSAGE_ORIGIN, Form("Event #%d : No hits found!", rawEvent -> GetEventID()));
    }
    else {
      event -> SetIsGood(kTRUE);
      fLogger->Info(MESSAGE_ORIGIN, 
                    Form("Event #%d : Reconstructed %d hits and %d clusters.",
                         rawEvent -> GetEventID(), event -> GetNumHits(), event -> GetNumClusters()));
    }
  }
}
