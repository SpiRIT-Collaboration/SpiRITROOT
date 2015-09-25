/**
 * @brief Hit-Track-Correlator using proximity arguments* 
 *
 * @atrhor Sebastian Neubert (TUM) -- original author
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 */

// SpiRITROOT classes
#include "STProximityHTCorrelator.hh"
#include "STHit.hh"
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"
#include "STDebugLogger.hh"

// ROOT classes
#include "TVector3.h"
#include "TMath.h"

#define SPEEDUP 5

STProximityHTCorrelator::STProximityHTCorrelator(Double_t cut, Double_t zStretch, Double_t helixcut)
{
  fProxCut = cut;
  fZStretch = zStretch;
  fMeanDist = 2.0;
  fHelixCut = 2.*helixcut;
}

Bool_t
STProximityHTCorrelator::Correlate(STRiemannTrack *track, 
                                   STRiemannHit *rhit, 
                                   Bool_t &survive, 
                                   Double_t &matchQuality)
{
  /**
   * Get the position of the hit of the given Riemann hit, rhit.
   * That is the position of the hit in TPC.
   * Riemann hit is the hit on Riemann sphere.
   */
  TVector3 posX = rhit -> GetHit() -> GetPosition();

  /**
   * When the track is already fitted, just check if the difference between the helical radius and
   * the distance from the helical center to hit position is bigger than fHelixCut.
   * In this case, matchQuality is the difference and the hit doesn't survive.
   */
  if (track -> IsFitted()) 
  {
    Double_t circDist = TMath::Abs((posX - track -> GetCenter()).Perp() - track -> GetR());
#ifdef DEBUGRIEMANNCUTS
    STDebugLogger::Instance() -> FillHist1Step("perp",(posX - track -> GetCenter()).Perp(),200,0,200);
    STDebugLogger::Instance() -> FillHist1Step("radius",track -> GetR(),200,0,200);
    STDebugLogger::Instance() -> FillHist1Step("diff",circDist,200,0,200);
#endif

    if (circDist > fHelixCut) {
      matchQuality = circDist;
      survive = kFALSE;
#ifdef DEBUGRIEMANNCUTS
      STDebugLogger::Instance() -> FillHist1Step("helixC_fail",circDist,10000,0,1000);
#endif
      return kTRUE;
    }
#ifdef DEBUGRIEMANNCUTS
    STDebugLogger::Instance() -> FillHist1("helixC",circDist,100,0,20);
    STDebugLogger::Instance() -> FillHist1Step("helixC_step",circDist,1000,0,1000);
#endif
  }

  UInt_t numHits = track -> GetNumHits();

  // Scale proxcut with track quality. This makes it looser for better defined tracks.
  Double_t proxcut = fProxCut;
  Double_t quality = track -> GetQuality(); // (0, 1]
  proxcut *= 1 + (3 * quality); // proxcut *= [1, 4]

  TVector3 pos, dis3;
  Double_t dis;
    
#ifdef DEBUGRIEMANNCUTS
   Double_t disMin = 1000000.;
   Bool_t survive_debug = kFALSE;
#endif

  // Check last and first hit for match
  for (UInt_t iHit = numHits - 1; kTRUE; iHit -= iHit) 
  {
    pos = track -> GetHit(iHit) -> GetHit() -> GetPosition();
    dis3 = posX - pos;
    dis3.SetZ(dis3.Z()/fZStretch); // What's this fZStretch for?
    dis = dis3.Mag();
#ifdef DEBUGRIEMANNCUTS
    if (dis < disMin) disMin = dis;
#endif

    if (dis < proxcut) {
      matchQuality = dis;
      survive = kTRUE;
#ifdef DEBUGRIEMANNCUTS
      survive_debug = kTRUE;
#else
      return kTRUE;
#endif
    }

    if (iHit == 0)
      break;
  }
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1("prox",disMin,100,0,30);
  STDebugLogger::Instance() -> FillHist1Step("prox_step",disMin,1000,0,1000);
  if (survive_debug == kTRUE)
    return kTRUE;
  else
    STDebugLogger::Instance() -> FillHist1Step("prox_fail",disMin,1000,0,1000);
#endif

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
  
  for (UInt_t iHit = 2; iHit < numHits - 1; iHit += SPEEDUP)
  {
    pos = track -> GetHit(iHit) -> GetHit() -> GetPosition();
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
  TVector3 dist = posX - track -> GetHit(track -> GetClosestHit(rhit, l, closest - SPEEDUP - 2, closest + SPEEDUP + 2)) -> GetHit() -> GetPosition();

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
