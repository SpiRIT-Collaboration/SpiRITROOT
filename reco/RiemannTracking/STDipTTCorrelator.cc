//-----------------------------------------------------------
// Description:
//      Track-Track-Correlator using dip fit
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Original Author List:
//      Sebastian Neubert    TUM            (original author)
//      Johannes Rauch       TUM
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STDipTTCorrelator.hh"

#include "STHitCluster.hh"
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"

// ROOT classes
#include "TVector3.h"
#include "TMath.h"

#include "STDebugLogger.hh"


STDipTTCorrelator::STDipTTCorrelator(Double_t proxcut, Double_t dipcut, Double_t helixcut)
{
  fProxCut = proxcut;
  fDipCut = dipcut;
  fHelixCut = helixcut;
}


Bool_t
STDipTTCorrelator::Correlate(STRiemannTrack* track1, STRiemannTrack* track2, Bool_t &survive, Double_t &matchQuality)
{
  if(!(track1 -> IsFitted()))
    return kFALSE;

  Double_t phi1, phi2;
  UInt_t numHits1 = track1 -> GetNumHits();
  UInt_t numHits2 = track2 -> GetNumHits();

  Double_t scaling = 20./numHits2;
  if      (scaling > 3.)  scaling = 3.;
  else if (scaling < 0.7) scaling = 0.7;

  //quick check, there is still an ambiguity (tracks might be sorted differently, so their dips could be symmetric around 90deg)
  if (track2 -> IsFitted()) {
    phi1 = track1 -> GetDip();
    phi2 = track2 -> GetDip();

    Double_t dphi1 = TMath::Abs(phi2 - phi1);
    Double_t dphi2;

    if (phi1 > phi2)
      dphi2 = TMath::Abs(phi2 + phi1 - TMath::Pi());
    else
      dphi2 = TMath::Abs(-phi2 - phi1 + TMath::Pi());

    if (dphi2 < dphi1)
      dphi1 = dphi2;

    if (dphi1 > fDipCut*scaling) {
      survive = kFALSE;
      return kTRUE;
    }
#ifdef DEBUGRIEMANNCUTS
    STDebugLogger::Instance() -> FillHist1("dipTT",dphi1,100,0,100);
#endif
  }

  // check distance
  TVector3 t1h1 = track1 -> GetFirstHit() -> GetCluster() -> GetPosition();
  TVector3 t1hn = track1 -> GetLastHit() -> GetCluster() -> GetPosition();
  TVector3 t2h1 = track2 -> GetFirstHit() -> GetCluster() -> GetPosition();
  TVector3 t2hn = track2 -> GetLastHit() -> GetCluster() -> GetPosition();

  Double_t d1n21 = (t1hn - t2h1).Mag();
  Double_t d1n2n = (t1hn - t2hn).Mag();
  Double_t d1121 = (t1h1 - t2h1).Mag();
  Double_t d112n = (t1h1 - t2hn).Mag();
  Double_t dist = d1n21;

  Bool_t back1 = kTRUE;
  Bool_t back2 = kFALSE;
  if (d1n2n < dist) {dist = d1n2n; back1 = kTRUE;  back2 = kTRUE;}
  if (d1121 < dist) {dist = d1121; back1 = kFALSE; back2 = kFALSE;}
  if (d112n < dist) {dist = d112n; back1 = kFALSE; back2 = kTRUE;}

  // check proximity
  if (dist > fProxCut) {
    survive = kFALSE;
    return kTRUE;
  }
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1("proxTT",dist,100,0,100);
#endif

  if (track2 -> IsFitted()) {
    TVector3 pos2, dir2;

    if (back2)
      track2 -> GetPosDirOnHelix(numHits2 - 1, pos2, dir2);
    else
      track2 -> GetPosDirOnHelix(0, pos2, dir2);


    // check if helix distance matches
    STHitCluster *testCluster = new STHitCluster();
    testCluster -> SetPosition(pos2);
    testCluster -> SetCharge(1.);
    STRiemannHit *testHit = new STRiemannHit(testCluster);
    Double_t hDist = track1 -> DistHelix(testHit, kTRUE, kTRUE);

    delete testHit;
    delete testCluster;

    // check if sz distace small enough
    Double_t scaling2 = scaling * dist/30.;

    if (scaling > 3)
      scaling = 3;

    scaling += 1;


    if (hDist > fHelixCut*scaling) {
      survive = kFALSE;
      return kTRUE;
    }
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1("helixTT",hDist,100,0,100);
#endif

    survive = kTRUE;
    return kTRUE;
  }

  // track2 not fitted: test hit by hit
  Double_t maxhDist = 0;
  for(UInt_t iHit = 0; iHit < track2 -> GetNumHits(); iHit++){
    Double_t hDist = TMath::Abs(track1 -> DistHelix(track2 -> GetHit(iHit), kTRUE));

    if (hDist > maxhDist)
      maxhDist = hDist;

    if (maxhDist > fHelixCut)
      break; // track did not survive!
  }

  matchQuality = maxhDist;

  if (maxhDist > fHelixCut) {
    survive = kFALSE;
    return kTRUE;
  }

  survive = kTRUE;
  return kTRUE;
}

