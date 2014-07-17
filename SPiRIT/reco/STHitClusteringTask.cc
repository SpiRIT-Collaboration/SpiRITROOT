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
#include "STEvent.hh"
#include "STHitSortY.hh"

// FairROOT classes
#include "FairRun.h"
#include "FairRuntimeDb.h"

// ROOT classes
#include "TVector3.h"

// STL
#include <vector>
#include <algorithm>

ClassImp(STHitClusteringTask)

STHitClusteringTask::STHitClusteringTask()
{
  fLogger = FairLogger::GetLogger();

  fIsPersistence = kFALSE;

  fEventHCArray = new TClonesArray("STEvent");
}

STHitClusteringTask::~STHitClusteringTask()
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
  if (fEventHArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEventH array!");

    return kERROR;
  }

  ioMan -> Register("STEventHC", "SPiRIT", fEventHCArray, fIsPersistence);

  fDriftLength = fPar -> GetDriftLength();
  fYDivider = fPar -> GetYDivider();

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
  fEventHCArray -> Delete();

  STEvent *eventH = (STEvent *) fEventHArray -> At(0);
  STEvent *eventHC = new ((*fEventHCArray)[0]) STEvent();

  Double_t sliceY = fDriftLength/fYDivider;

  std::vector<STHit> *hitArray = eventH -> GetHitArray();
  std::sort(hitArray -> begin(), hitArray -> end(), STHitSortY());

  std::vector<STHit> slicedSpace;
  for (Int_t iSlice = 0, currentPosInVector = 0; iSlice < fYDivider; iSlice++) {
    Double_t bottomY = -fDriftLength + iSlice*sliceY;
    Double_t topY = bottomY + sliceY;

    slicedSpace.clear();
    while (currentPosInVector < hitArray -> size()) {
      Double_t yPos = (hitArray -> at(currentPosInVector)).GetPosition().Y();

      if (yPos >= bottomY && yPos < topY) {
        slicedSpace.push_back(hitArray -> at(currentPosInVector));

        currentPosInVector++;
      } else
        break;
    }

    if (slicedSpace.empty())
      continue;

    // Now I'll write cluster finding part.
  }
}
