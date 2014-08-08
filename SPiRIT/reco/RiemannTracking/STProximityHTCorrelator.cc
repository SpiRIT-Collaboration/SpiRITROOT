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

//#include "DebugLogger.h"

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
  TVector3 posX = rhit -> GetCluster() -> GetPosition();

  // fast estimation: distance to circle in 2D
  if (track -> IsFitted()) {
    Double_t circDist = TMath::Abs((posX - track -> GetCenter()).Perp() - track -> GetR());
    if (circDist > fHelixCut) {
      matchQuality = circDist;
      survive = kFALSE;
      return kTRUE;
    }
  }

  UInt_t numHits = track -> GetNumHits();

  //scale proxcut with track quality (makes it looser for better defined tracks)
  Double_t proxcut = fProxCut;
  Double_t quality = track -> GetQuality();
  proxcut *= 1 + (3 * quality);

  TVector3 pos, dis3;
  Double_t dis;
    
  // check last and first hit for match
  for (UInt_t iHit = numHits - 1; kTRUE; iHit -= iHit){
    pos = track -> GetHit(iHit) -> GetCluster() -> GetPosition();
    dis3 = posX - pos;
    dis3.SetZ(dis3.Z()/fZStretch);
    dis = dis3.Mag();

    if (dis < proxcut) {
      matchQuality = dis;
      survive = kTRUE;
      return kTRUE;
    }

    if (iHit == 0)
      break;
  }
  
  if (numHits < 3) { // the hit (numHits = 1), resp. both hits  (numHits = 2) have been checked and did not survive
    matchQuality = dis;
    survive = kFALSE;
    return kTRUE;
  }

  Int_t closest = -1;

  // now check every SPEEDUP hit    
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

  if (closest == -1) { // no hit closer than largecut has been found
    matchQuality = mindis;
    survive = kFALSE;
    return kTRUE;
  }

  Double_t l;
  TVector3 dist = posX - track -> GetHit(track -> GetClosestHit(rhit, l, closest - SPEEDUP - 2, closest + SPEEDUP + 2)) -> GetCluster() -> GetPosition();

  matchQuality = l; // unaltered distance in 3D

  dist.SetZ(dist.Z()/fZStretch);
  l = dist.Mag();

//  DebugLogger::Instance() -> Histo("HT_prox_l",l,0,5,100);
  if (l > proxcut) {
//    DebugLogger::Instance() -> Histo("HT_riemanncuts",1,0,20,20);
    survive = kFALSE;
    return kTRUE;
  }
  survive = kTRUE;
  //std::cout<<"passed\n";
  return kTRUE;
}
