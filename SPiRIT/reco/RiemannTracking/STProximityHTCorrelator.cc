//-----------------------------------------------------------
// Description:
//      Hit-Track-Correlator using proximity arguments
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Original Author List:
//      Sebastian Neubert    TUM            (original author)
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STProximityHTCorrelator.hh"
#include "STHitCluster.hh"
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"

// ROOT classes
#include "TVector3.h"
#include "TMath.h"

#define SPEEDUP 5

STProximityHTCorrelator::STProximityHTCorrelator(Double_t cut, Double_t zStretch, Double_t helixcut)
{
  fProxCut = cut;
  fZStretch = zStretch;
  fMeanDist = 0.7;
  fHelixCut = 2.*helixcut;
}

Bool_t
STProximityHTCorrelator::Correlate(STRiemannTrack *track, STRiemannHit *rhit, Bool_t &survive, Double_t &matchQuality)
{
  // Get the position of the cluster of the given Riemann hit, rhit.
  // That is the position of the hit in TPC.
  // Riemann hit is the hit on Riemann sphere.
  TVector3 posX = rhit -> GetCluster() -> GetPosition();

  // When the track is already fitted, just check if the difference between the helical radius and
  // the distance from the helical center to hit position is bigger than fHelixCut.
  // In this case, matchQuality is the difference and the hit doesn't survive.
  if (track -> IsFitted()) {
    Double_t circDist = TMath::Abs((posX - track -> GetCenter()).Perp() - track -> GetR());

    if (circDist > fHelixCut) {
      matchQuality = circDist;
      survive = kFALSE;
      return kTRUE;
    }
  }

  UInt_t numHits = track -> GetNumHits();

  // Scale proxcut with track quality. This makes it looser for better defined tracks.
  Double_t proxcut = fProxCut;
  Double_t quality = track -> GetQuality(); // (0, 1]
  proxcut *= 1 + (3 * quality); // proxcut *= [1, 4]

  TVector3 pos, dis3;
  Double_t dis;
    
  // Check last and first hit for match
  for (UInt_t iHit = numHits - 1; iHit < 0; iHit--) {
    pos = track -> GetHit(iHit) -> GetCluster() -> GetPosition();
    dis3 = posX - pos;
    dis3.SetZ(dis3.Z()/fZStretch); // What's this fZStretch for?
    dis = dis3.Mag();

    if (dis < proxcut) {
      matchQuality = dis;
      survive = kTRUE;
      return kTRUE;
    }
  }

  // The hit (numHits = 1), resp. both hits  (numHits = 2) have been checked and did not survive
  if (numHits < 3) {
    matchQuality = dis;
    survive = kFALSE;
    return kTRUE;
  }

  Int_t closest = -1;

  // Now check every SPEEDUP hit    
  Double_t largecut = 0.6*SPEEDUP*fMeanDist + proxcut*fZStretch;
  Double_t mindis = 1.E10;
  
  for (UInt_t iHit = 2; iHit < numHits - 1; iHit += SPEEDUP){
    pos = track -> GetHit(iHit) -> GetCluster() -> GetPosition();
    dis = (posX - pos).Mag();
    if (dis < mindis) {
      mindis = dis;
      if (mindis < largecut)
        closest = iHit;

      if (mindis < proxcut && track -> IsFitted()) {
        matchQuality = mindis;
        survive = kTRUE;
        return kTRUE;
      }
    }
  }

  // No hit closer than largecut has been found
  if (closest == -1) {
    matchQuality = mindis;
    survive = kFALSE;
    return kTRUE;
  }

  Double_t l;
  TVector3 dist = posX - track -> GetHit(track -> GetClosestHit(rhit, l, closest - SPEEDUP - 2, closest + SPEEDUP + 2)) -> GetCluster() -> GetPosition();

  matchQuality = l; // unaltered distance in 3D

  dist.SetZ(dist.Z()/fZStretch);
  l = dist.Mag();

  if (l > proxcut) {
    survive = kFALSE;
    return kTRUE;
  }

  survive = kTRUE;
  return kTRUE;
}
