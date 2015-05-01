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

// FairROOT classes
#include "FairRun.h"
#include "FairRuntimeDb.h"

// ROOT classes
#include "TVector3.h"

// STL
#include <algorithm>

using std::vector;

ClassImp(STHitClusteringTask);

STHitClusteringTask::STHitClusteringTask()
:FairTask("SpiRIT Hit Clusterint Task")
{
  fLogger = FairLogger::GetLogger();

  fIsPersistence = kFALSE;

  fEventHCArray = new TClonesArray("STEvent");
}

STHitClusteringTask::~STHitClusteringTask()
{
}

void STHitClusteringTask::SetPersistence(Bool_t value) { fIsPersistence = value; }
void STHitClusteringTask::SetVerbose(Int_t value)      { fVerbose = value; }

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
  STEvent *eventHC = (STEvent *) new ((*fEventHCArray)[0]) STEvent();

  if (!(eventH -> IsGood())) {
    eventHC -> SetIsGood(kFALSE);

    return;
  }

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
  /**
    * In the passed sliced space, **slicedSpace**, find hit clusters and store it and clustered hits into passed STEvent object, **event**.
    * When adding hits to STHitCluster iobject, weighted average calculation is done to determine the cluster position.
   **/

  if (fVerbose == 1)
    fLogger -> Info(MESSAGE_ORIGIN, "Start cluster finding!");

  for (Int_t iHit = 0; iHit < slicedSpace.size(); iHit++) {
    STHit *centerHit = &(slicedSpace.at(iHit));

    if (centerHit -> IsClustered())
      continue;

    vector<Int_t> clusteredHits;
    centerHit = FindLargestHitAndCloseHits(slicedSpace, centerHit, clusteredHits);

    STHitCluster *cluster = new STHitCluster();
    for (Int_t iNum = 0; iNum < clusteredHits.size(); iNum++) {
      Int_t hitNum = clusteredHits.at(iNum);
      STHit *hit = &(slicedSpace.at(hitNum));
      hit -> SetIsClustered(kTRUE);
      hit -> SetClusterID(event -> GetNumClusters());

      cluster -> AddHit(hit);
      event -> AddHit(hit);
    }
    
    cluster -> SetClusterID(event -> GetNumClusters());
    event -> AddCluster(cluster);
    delete cluster;
    iHit = 0;
  }
}

STHit *
STHitClusteringTask::FindLargestHitAndCloseHits(vector<STHit> &slicedSpace, STHit *centerHit, vector<Int_t> &clusteredHits)
{
  /**
    * In the passed sliced space, **slicedSpace**, find the hit having the largest charge in surrounding 8 pads with center, **centerHit**.
    * If there is larger charged hit in neighbor, move the center to it and start over.
    * Finally, found hits are stored in **clusteredHits**.
   **/

  if (fVerbose > 0)
    fLogger -> Info(MESSAGE_ORIGIN, Form("Start to find the largest hit and close hits with center Hit:%d in slicedSpace:%lu", centerHit -> GetHitID(), slicedSpace.size()));

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

  clusteredHits.clear();
  for (Int_t iHit = 0; iHit < slicedSpace.size(); iHit++) {
    STHit *hit = &(slicedSpace.at(iHit));

    if (fVerbose > 1)
      fLogger -> Info(MESSAGE_ORIGIN, Form("HitID: %d", hit -> GetHitID()));

    if (hit -> IsClustered())
      continue;

    Double_t xPos = (hit -> GetPosition()).X();
    Double_t zPos = (hit -> GetPosition()).Z();

    if (minX < xPos && xPos < maxX && minZ < zPos && zPos < maxZ)
      clusteredHits.push_back(iHit);
  }

  if (fVerbose > 1)
    fLogger -> Info(MESSAGE_ORIGIN, Form("Found clusteredHits %lu", clusteredHits.size()));

  if (clusteredHits.size() == 1)
    return centerHit;

  for (Int_t iHit = 0; iHit < clusteredHits.size(); iHit++) {
    Int_t hitNumber = clusteredHits.at(iHit);
    STHit *hit = &(slicedSpace.at(hitNumber));
    if (centerHit -> GetCharge() < hit -> GetCharge())
      return FindLargestHitAndCloseHits(slicedSpace, hit, clusteredHits);
  }

  return centerHit;
}
