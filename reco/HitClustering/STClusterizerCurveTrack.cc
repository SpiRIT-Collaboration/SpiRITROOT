#include "STClusterizerCurveTrack.hh"
#include "STCurveTrack.hh"

STClusterizerCurveTrack::STClusterizerCurveTrack()
{
  fClusterArray = new TClonesArray("STHitCluster", 50);
  fTrackFitter = new STCurveTrackFitter();
  fTrackFitter -> SetNumHitsAtHead(fNumHitsAtHead);
  fRiemannFitter = new STRiemannFitter();
  fTracker = new STCurveTrack();
}

STClusterizerCurveTrack::~STClusterizerCurveTrack()
{
}

void 
STClusterizerCurveTrack::AnalyzeTrack(TClonesArray* trackArray, STEvent* eventOut)
{
  Int_t numTracks = trackArray -> GetEntries();
  Int_t clusterID = 0;

  for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
  {
    fClusterArray -> Clear();

    STCurveTrack *track = (STCurveTrack*) trackArray -> At(iTrack);
    AnalyzeSingleTrack(track, fClusterArray);

    Int_t numClusters = fClusterArray -> GetEntriesFast();
    for (Int_t iCluster = 0; iCluster < numClusters; iCluster++)
    {
      STHitCluster *cluster = (STHitCluster *) fClusterArray -> At(iCluster);

      Double_t x = (cluster -> GetPosition()).X();
      if (x > fXHighCut || x < fXLowCut)
        continue;

      cluster -> SetHitID(clusterID);
      cluster -> SetClusterID(clusterID);
      track -> AddCluster(cluster);
      eventOut -> AddCluster(cluster);

      clusterID++;
    }
  }
}

void 
STClusterizerCurveTrack::AnalyzeTrack(TClonesArray *trackArray, TClonesArray *clusterArray)
{
  Int_t numTracks = trackArray -> GetEntries();

  Int_t index = 0;

  for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
  {
    fClusterArray -> Clear();

    STCurveTrack *track = (STCurveTrack*) trackArray -> At(iTrack);
    AnalyzeSingleTrack(track, fClusterArray);

    Int_t numClusters = fClusterArray -> GetEntriesFast();
    for (Int_t iCluster = 0; iCluster < numClusters; iCluster++)
    {
      STHitCluster *cluster = (STHitCluster *) fClusterArray -> At(iCluster);

      Double_t x = (cluster -> GetPosition()).X();
      if (x > fXHighCut || x < fXLowCut) {
        continue;
      }

      cluster -> SetClusterID(index);
      STHitCluster *in = new ((*clusterArray)[index]) STHitCluster(cluster);
      in -> SetClusterID(index);
      track -> AddCluster(in);

      index++;
    }
  }
}

void 
STClusterizerCurveTrack::AnalyzeSingleTrack(STCurveTrack *track, TClonesArray *clusterArray)
{
  Int_t numHitsInTrack = track -> GetNumHits();
  std::vector<STHit*> *hitArrayFromTrack = track -> GetHitPointerArray();
  STHit* hit = nullptr;
  STHitCluster *cluster;

  if (numHitsInTrack <= 5) 
  {
    hit = hitArrayFromTrack -> at(0);
    cluster = NewCluster(hit, clusterArray);

    for (Int_t iHit = 1; iHit < numHitsInTrack; iHit++) {
      hit = hitArrayFromTrack -> at(iHit);
      cluster -> AddHit(hit);
    }

    return;
  }

  if (fClusteringOption == 0) // Cluster by layer
  {
    STHitSortLayer sorting;
    std::sort(hitArrayFromTrack -> begin(), hitArrayFromTrack -> end(), sorting);

    hit = hitArrayFromTrack -> at(0);
    Int_t currentLayer = hit -> GetLayer();
    cluster = NewCluster(hit, clusterArray);

    for (Int_t iHit = 1; iHit < numHitsInTrack; iHit++)
    {
      hit = hitArrayFromTrack -> at(iHit);
      if (hit -> GetLayer() != currentLayer)
      {
        currentLayer = hit -> GetLayer();
        cluster = NewCluster(hit, clusterArray);
      }
      cluster -> AddHit(hit);
    }
  }
  else
  {
    Double_t xCenter = 0, zCenter = 0, radius = 0, rms = 0, rmsP = 0;
    Bool_t fitted = fRiemannFitter -> FitData(hitArrayFromTrack, xCenter, zCenter, radius, rms, rmsP);

    if (fitted == kTRUE && rms < 12)
    {
      if (radius == 0) {
        STHitSortDirection sorting(TVector3(xCenter, 0, zCenter));
        std::sort(hitArrayFromTrack -> begin(), hitArrayFromTrack -> end(), sorting);
      }
      else {
        STHitSortThetaFromP sorting(TVector3(xCenter, 0, zCenter));
        std::sort(hitArrayFromTrack -> begin(), hitArrayFromTrack -> end(), sorting);
      }
    }
    else
    {
      fTrackFitter -> FitAndSetTrack(track);
      STHitSortDirection sorting(track -> GetDirection());
      std::sort(hitArrayFromTrack -> begin(), hitArrayFromTrack -> end(), sorting);
    }

    STHit *preHit = track -> GetHit(0);
    cluster = NewCluster(preHit, clusterArray);
    STHit *curHit = nullptr;

    Int_t numHitsAtHead = fNumHitsAtHead;
    if (numHitsAtHead > numHitsInTrack)
      numHitsAtHead = numHitsInTrack;

    fTracker -> Clear();
    for (Int_t iHit = 0; iHit < numHitsAtHead; iHit++) {
      hit = hitArrayFromTrack -> at(iHit);
      fTracker -> AddHit(hit);
    }
    fTrackFitter -> FitAndSetCurveTrack(fTracker);

    for (Int_t iHit = 1; iHit < numHitsInTrack; iHit++)
    {
      curHit = hitArrayFromTrack -> at(iHit);
      if (iHit >= numHitsAtHead)
      {
        fTracker -> AddHit(curHit);
        fTrackFitter -> FitAndSetCurveTrack(fTracker);
      }
      TVector3 direction = fTracker -> GetDirection();

      Double_t length = std::abs(preHit -> GetPosition().Dot(direction) - curHit -> GetPosition().Dot(direction));

      if (length > 12)
      {
        cluster = NewCluster(curHit, clusterArray);
        preHit = curHit;
      }
      else {
        cluster -> AddHit(curHit);
      }
    }
  }
}

STHitCluster*
STClusterizerCurveTrack::NewCluster(STHit* hit, TClonesArray *array)
{
  Int_t index = array -> GetEntriesFast();
  STHitCluster* cluster = new ((*array)[index]) STHitCluster();
  cluster -> SetClusterID(index);
  cluster -> AddHit(hit);

  return cluster;
}
