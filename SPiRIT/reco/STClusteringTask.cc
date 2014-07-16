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
#include "STClusteringTask.hh"

ClassImp(STClusteringTask)

STClusteringTask()::STClusteringTask()
{
  fLogger = FairLogger::GetLogger();

  fIsPersistence = kFALSE;

  fEventC_Array = new TClonesArray("STEvent");
}

STClusteringTask()::~STClusteringTask()
{
}

void STClusteringTask::SetPersistence(Bool_t value) { fIsPersistence = value; }

InitStatus
STClusteringTask::Init()
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
STClusteringTask::SetParContainers()
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
STClusteringTask::Exec(Option_t *opt)
{
}
