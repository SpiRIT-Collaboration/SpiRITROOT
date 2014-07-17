//-----------------------------------------------------------
// Description:
//   Clustering hits processed by PSATask
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STHitClusteringTask.hh"

ClassImp(STHitClusteringTask)

STHitClusteringTask()::STHitClusteringTask()
{
  fLogger = FairLogger::GetLogger();

  fIsPersistence = kFALSE;

  fEventHCArray = new TClonesArray("STEvent");
}

STHitClusteringTask()::~STHitClusteringTask()
{
}

void STHitClusteringTask::SetPersistence(Bool_t value) { fIsPersistence = value; }

InitStatus
STHitClusteringTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");

    return kERROR;
  }

  fEventHArray = (TClonesArray *) ioMan -> GetObject("STEventH");
  if (fEvent__Array == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEventH array!");

    return kERROR;
  }

  ioMan -> Register("STEventHC", "SPiRIT", fEventHCArray, fIsPersistence);

  return kSUCCESS;
}

void
STHitClusteringTask::SetParContainers()
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
STHitClusteringTask::Exec(Option_t *opt)
{
}
