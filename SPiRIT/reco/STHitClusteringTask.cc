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
#include <algorithm>

using std::vector;

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

  vector<STHit> *hitArray = eventH -> GetHitArray();
  std::sort(hitArray -> begin(), hitArray -> end(), STHitSortY());

  vector<STHit> slicedSpace;
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
    
    FindCluster(slicedSpace, eventHC); 
  }

  eventHC -> SetIsClustered(kTRUE);
}

void
STHitClusteringTask::FindCluster(vector<STHit> &slicedSpace, STEvent *event)
{
  for (Int_t iHit = 0; iHit < slicedSpace.size(); iHit++) {
    STHit *centerHit = &(slicedSpace.at(iHit));

    if (centerHit -> GetIsClustered())
      continue;

    vector<Int_t> closeHits;
    centerHit = FindLargestHitAndCloseHits(slicedSpace, centerHit, closeHits);

    STHitCluster *cluster = new STHitCluster();
    for (Int_t iNum = 0; iNum < closeHits.size(); iNum++) {
      STHit *hit = &(slicedSpace.at(iNum));
      hit -> SetIsClustered(kTRUE);
      hit -> SetClusterID(event -> GetNumClusters());

      cluster -> AddHit(hit);
      event -> AddHit(hit);
    }

    event -> AddCluster(cluster);
    delete cluster;
    iHit = 0;
  }
}

STHit *
STHitClusteringTask::FindLargestHitAndCloseHits(vector<STHit> &slicedSpace, STHit *centerHit, vector<Int_t> &closeHits)
{
  Int_t padSizeX = fPar -> GetPadSizeX();
  Int_t padSizeZ = fPar -> GetPadSizeZ();
  Int_t padPlaneX = fPar -> GetPadPlaneX();
  Int_t padPlaneZ = fPar -> GetPadPlaneZ();

  Double_t minX = (centerHit -> GetPosition()).X() - 1.5*padSizeX;
  Double_t maxX = (centerHit -> GetPosition()).X() + 1.5*padSizeX;
  Double_t minZ = (centerHit -> GetPosition()).Z() - 1.5*padSizeZ;
  Double_t maxZ = (centerHit -> GetPosition()).Z() + 1.5*padSizeZ;

  if (minX < -padPlaneX) minX = -padPlaneX;
  if (maxX >  padPlaneX) maxX =  padPlaneX;
  if (minZ <  0)         minZ = 0;
  if (maxZ >  padPlaneZ) maxZ = padPlaneZ;

  closeHits.clear();
  for (Int_t iHit = 0; iHit < slicedSpace.size(); iHit++) {
    STHit *hit = &(slicedSpace.at(iHit));

    if (hit -> GetHitID() == centerHit -> GetHitID() || hit -> GetIsClustered())
      continue;

    Double_t xPos = (hit -> GetPosition()).X();
    Double_t zPos = (hit -> GetPosition()).Z();

    if (minX < xPos && xPos < maxX && minZ < zPos && zPos < maxZ)
      closeHits.push_back(iHit);
  }

  if (closeHits.size() == 0)
    return centerHit;
    
  for (Int_t iHit = 0; iHit < closeHits.size(); iHit++) {
    Int_t hitNumber = closeHits.at(iHit);
    STHit *hit = &(slicedSpace.at(hitNumber));
    if (centerHit -> GetCharge() < hit -> GetCharge())
      return FindLargestHitAndCloseHits(slicedSpace, hit, closeHits);
  }

  return centerHit;
}
