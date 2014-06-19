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
  fNumTbs = 512;

  fPar = NULL;
}

STDecoderTask::~STDecoderTask()
{
}

void
STDecoderTask::SetNumTbs(Int_t numTbs)
{
  fNumTbs = numTbs;
}

InitStatus
STDecoderTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    Error("STDecoderTask::Init()", "RootManager not instantiated!");
    return kERROR;
  }

  Char_t *grawFile = (Char_t *) ioMan -> GetObject("GRAWInputFileName");
  if (grawFile == NULL) {
    Error("STDecoderTask::Init()", "GRAW file not found!");
    return kERROR;
  }

  fDecoder = new GETDecoder(grawFile, fNumTbs);

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
}
