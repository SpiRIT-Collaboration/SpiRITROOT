//-----------------------------------------------------------
// Description:
//   Converting GRAW file to tree structure to make it easy
//   to access the data
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#include "STDecoderTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

ClassImp(STDecoderTask);

STDecoderTask::STDecoderTask()
{
  fDecoder = NULL;

  fGrawFile = NULL;
  fPedestalFile = NULL;
  fNumTbs = 512;

  fPar = NULL;
  fRawEventArray = new TClonesArray("STRawEvent");
}

STDecoderTask::~STDecoderTask()
{
}

void
STDecoderTask::SetNumTbs(Int_t numTbs)
{
  fNumTbs = numTbs;
}

void
STDecoderTask::SetGraw(Char_t *filename)
{
  fGrawFile = filename;
}

void
STDecoderTask::SetPedestal(Char_t *filename)
{
}

InitStatus
STDecoderTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    Error("STDecoderTask::Init()", "RootManager not instantiated!");
    return kERROR;
  }

  ioMan -> Register("STRawEvent", "SPiRIT", fRawEventArray, 1);

  fDecoder = new STCore(fGrawFile, fNumTbs);
  fDecoder -> SetUAMap((fPar -> GetFile(0)).Data());
  fDecoder -> SetAGETMap((fPar -> GetFile(1)).Data());
  if (!fPedestalFile) {
    Info("STDecoderTask::Init()", "Use internal pedestal!");

    fDecoder -> SetInternalPedestal();
  } else
    fDecoder -> SetPedestalData(fPedestalFile);

  return kSUCCESS;
}

void
STDecoderTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    Fatal("STDecoderTask::SetParContainers()", "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    Fatal("STDecoderTask::SetParContainers()", "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    Fatal("STDecoderTask::SetParContainers()", "STDigiPar not found!!");

}

void
STDecoderTask::Exec(Option_t *opt)
{
  STRawEvent *rawEvent = fDecoder -> GetRawEvent();
  
  Int_t lastIdx = fRawEventArray -> GetEntriesFast();
  new ((*fRawEventArray)[lastIdx]) STRawEvent(rawEvent);
}
