/**
 * @brief Track-Track-Correlator using dip fit
 *
 * @author Sebastian Neubert (TUM) -- original author
 * @author Johannes Rauch    (TUM)
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 */

// SpiRITROOT classes
#include "STDipTTCorrelator.hh"

#include "STHit.hh"
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"
#include "STDebugLogger.hh"

// ROOT classes
#include "TVector3.h"
#include "TMath.h"

STDipTTCorrelator::STDipTTCorrelator(Double_t proxcut, Double_t dipcut, Double_t helixcut)
{
  fProxCut = proxcut;
  fDipCut = dipcut;
  fHelixCut = helixcut;
}


Bool_t
STDipTTCorrelator::Correlate(STRiemannTrack* track1, 
                             STRiemannTrack* track2, 
                             Bool_t &survive, 
                             Double_t &matchQuality)
{
  if ( !(track1 -> IsFitted()) )
    return kFALSE;

  UInt_t numHits1 = track1 -> GetNumHits();
  UInt_t numHits2 = track2 -> GetNumHits();

  Double_t scaling = 20./numHits2;
  if      (scaling > 3.)  scaling = 3.;
  else if (scaling < 0.7) scaling = 0.7;

  // quick check, there is still an ambiguity 
  // tracks might be sorted differently, so their dips could be symmetric around 90deg
  if (track2 -> IsFitted()) 
  {
    Double_t phi1  = track1 -> GetDip();
    Double_t phi2  = track2 -> GetDip();
    Double_t phiDiff1 = TMath::Abs(phi2 - phi1);

    Double_t phiDiff2;
    if (phi1 > phi2) phiDiff2 = TMath::Abs(phi2 + phi1 - TMath::Pi());
    else             phiDiff2 = TMath::Abs(-phi2 - phi1 + TMath::Pi());

    if (phiDiff2 < phiDiff1) phiDiff1 = phiDiff2;

#ifdef SUBTASK_RIEMANN
    STDebugLogger::InstanceX() -> Print("DipTT", 
      Form("phi diff: %f, dip cut: %f", phiDiff1, fDipCut*scaling));
#endif
    if (phiDiff1 > fDipCut*scaling) 
    {
      survive = kFALSE;
#ifdef DEBUGRIEMANNCUTS
      STDebugLogger::Instance() -> FillHist1Step("dip_fail",phiDiff1,200,0,200);
#endif
      return kTRUE;
    }
#ifdef DEBUGRIEMANNCUTS
    STDebugLogger::Instance() -> FillHist1("dip",phiDiff1,100,0,20);
#endif
  }

  // check distance
  TVector3 t1h1 = track1 -> GetFirstHit() -> GetHit() -> GetPosition();
  TVector3 t1hn = track1 -> GetLastHit()  -> GetHit() -> GetPosition();
  TVector3 t2h1 = track2 -> GetFirstHit() -> GetHit() -> GetPosition();
  TVector3 t2hn = track2 -> GetLastHit()  -> GetHit() -> GetPosition();

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

#ifdef SUBTASK_RIEMANN
  STDebugLogger::InstanceX() -> Print("DipTT", 
    Form("prox: %f, prox cut: %f", dist, fProxCut));
#endif
  // check proximity
  if (dist > fProxCut) 
  {
    survive = kFALSE;
#ifdef DEBUGRIEMANNCUTS
    STDebugLogger::Instance() -> FillHist1Step("proxT_fail",dist,200,0,200);
#endif
    return kTRUE;
  }
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1("proxT",dist,100,0,20);
#endif

  if (track2 -> IsFitted()) 
  {
    TVector3 pos2, dir2;

    if (back2) track2 -> GetPosDirOnHelix(numHits2 - 1, pos2, dir2);
    else       track2 -> GetPosDirOnHelix(0, pos2, dir2);


    // check if helix distance matches
    STHit *testHit = new STHit();
    testHit -> SetPosition(pos2);
    testHit -> SetCharge(1.);
    STRiemannHit *testRiemannHit = new STRiemannHit(testHit);
    Double_t hDist = track1 -> DistHelix(testRiemannHit, kTRUE, kTRUE);

    delete testRiemannHit;
    delete testHit;

    // check if sz distace small enough
    Double_t scaling2 = scaling * dist/30.;

    if (scaling > 3)
      scaling = 3;

    scaling += 1;


#ifdef SUBTASK_RIEMANN
  STDebugLogger::InstanceX() -> Print("DipTT", 
    Form("dist: %f, helix cut: %f", hDist, fHelixCut*scaling));
#endif
    if (hDist > fHelixCut*scaling) {
      survive = kFALSE;
#ifdef DEBUGRIEMANNCUTS
      STDebugLogger::Instance() -> FillHist1Step("helixT_fail",hDist,200,0,200);
#endif
      return kTRUE;
    }
#ifdef DEBUGRIEMANNCUTS
    STDebugLogger::Instance() -> FillHist1("helixT",hDist,100,0,20);
#endif

    survive = kTRUE;
    return kTRUE;
  }

  // track2 not fitted: test hit by hit
  Double_t maxhDist = 0;
  for(UInt_t iHit = 0; iHit < track2 -> GetNumHits(); iHit++)
  {
    Double_t hDist = TMath::Abs(track1 -> DistHelix(track2 -> GetHit(iHit), kTRUE));

    if (hDist > maxhDist)
      maxhDist = hDist;

    if (maxhDist > fHelixCut)
      break; // track did not survive!
  }

  matchQuality = maxhDist;

#ifdef SUBTASK_RIEMANN
  STDebugLogger::InstanceX() -> Print("DipTT", 
    Form("max dist: %f, helix cut: %f", maxhDist, fHelixCut));
#endif
  if (maxhDist > fHelixCut) 
  {
    survive = kFALSE;
    return kTRUE;
  }

  survive = kTRUE;
  return kTRUE;
}

