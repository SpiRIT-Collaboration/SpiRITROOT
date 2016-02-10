#include "STClusterizerLinearTrack.hh"
#include "STLinearTrack.hh"
#include <iostream>
using namespace std;

STClusterizerLinearTrack::STClusterizerLinearTrack()
{
  fClusterArray = new TClonesArray("STHitCluster", 50);

  fLinearTrackFitter = new STLinearTrackFitter();
}

STClusterizerLinearTrack::~STClusterizerLinearTrack()
{
}

void 
STClusterizerLinearTrack::AnalyzeTrack(TClonesArray* trackArray, STEvent* eventOut)
{
  fClusterArray -> Clear();

  Int_t numTracks = trackArray -> GetEntries();

  for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
  {
    STLinearTrack *track = (STLinearTrack*) trackArray -> At(iTrack);
    AnalyzeSingleTrack(track, fClusterArray);
  }

  Int_t n = fClusterArray -> GetEntriesFast();
  for (Int_t iCluster = 0; iCluster < n; iCluster++)
  {
    STHitCluster *cluster = (STHitCluster *) fClusterArray -> At(iCluster);
    eventOut -> AddCluster(cluster);
  }
}

void 
STClusterizerLinearTrack::AnalyzeSingleTrack(STLinearTrack *track, TClonesArray *clusterArray)
{
  std::vector<STHit*> *hitArrayFromTrack = track -> GetHitPointerArray();

  Int_t numHits = hitArrayFromTrack -> size();

  STHit* hit = hitArrayFromTrack -> at(0);
  STHitCluster *cluster = NewCluster(hit, clusterArray);

  Double_t l1 = fLinearTrackFitter -> GetLengthOnTrack(track, hit);
  Double_t l2 = l1;

  for (Int_t iHit = 1; iHit < numHits; iHit++)
  {
    hit = hitArrayFromTrack -> at(iHit);
    l2 = fLinearTrackFitter -> GetLengthOnTrack(track, hit);

    Double_t d = l1 - l2;

    if (d > 12) {
      cluster = NewCluster(hit, clusterArray);
      l1 = l2;
    }
    else {
      cluster -> AddHit(hit);
    }
  }
}

STHitCluster*
STClusterizerLinearTrack::NewCluster(STHit* hit, TClonesArray *array)
{
  Int_t index = array -> GetEntriesFast();
  STHitCluster* cluster = new ((*array)[index]) STHitCluster();
  cluster -> SetClusterID(index);
  cluster -> AddHit(hit);

  return cluster;
}
