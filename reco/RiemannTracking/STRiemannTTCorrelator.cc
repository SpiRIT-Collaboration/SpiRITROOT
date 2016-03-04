/**
 * @brief Track-Track-Correlator using riemann fit
 *
 * @author Sebastian Neubert (TUM) -- original author
 * @author Johannes Rauch    (TUM)
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 */

// SpiRITROOT classes
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"
#include "STRiemannTTCorrelator.hh"

// ROOT classes
#include "TVector3.h"
#include "TMath.h"

#include "STDebugLogger.hh"

STRiemannTTCorrelator::STRiemannTTCorrelator(Double_t planecut, Int_t minHitsForFit)
{
  fPlaneCut = planecut;
  fMinHitsForFit = minHitsForFit;
}

Bool_t
STRiemannTTCorrelator::Correlate(STRiemannTrack *track1, 
                                 STRiemannTrack *track2, 
                                 Bool_t &survive, 
                                 Double_t &matchQuality)
{
  // check scale
  if (track1 -> GetRiemannScale() != track2 -> GetRiemannScale()) {
    survive = kFALSE;
    return kFALSE;
  }

  UInt_t numHits1 = track1 -> GetNumHits();
  UInt_t numHits2 = track2 -> GetNumHits();
  UInt_t nhits = numHits1+numHits2;

  if (nhits < fMinHitsForFit)
    return kFALSE;

  // fill hits into new RiemannTrack and refit
  STRiemannTrack *mergedTrack = new STRiemannTrack(track1 -> GetRiemannScale());
  mergedTrack -> SetSort(kFALSE); // it's faster and we don't need it anyway

  for (UInt_t iHit = 0; iHit < numHits1; iHit++) {
    STRiemannHit *hit = new STRiemannHit(*(track1 -> GetHit(iHit)));
    mergedTrack -> AddHit(hit);
  }

  for (UInt_t iHit = 0; iHit < numHits2; iHit++) {
    STRiemannHit *hit = new STRiemannHit(*(track2 -> GetHit(iHit)));
    mergedTrack -> AddHit(hit);
  }

  mergedTrack -> FitAndSort();

  Double_t rms = mergedTrack -> DistRMS();
  Double_t sinDip = mergedTrack -> GetSinDip();
  Double_t scaling = 0.5 + 0.5*sinDip;

  mergedTrack -> DeleteHits();
  delete mergedTrack;

  matchQuality = rms;

#ifdef SUBTASK_RIEMANN
  STDebugLogger::InstanceX() -> Print("RiemannTT", 
    Form("rms: %f, plane cut: %f", rms, fPlaneCut*scaling));
  STDebugLogger::InstanceX() -> Print("RiemannTT", 
    Form("track-1 rms: %f", track1 -> DistRMS()));
  STDebugLogger::InstanceX() -> Print("RiemannTT", 
    Form("track-2 rms: %f", track2 -> DistRMS()));
#endif
  if (rms > fPlaneCut*scaling) 
  {
    survive = kFALSE;
#ifdef DEBUGRIEMANNCUTS
    STDebugLogger::Instance() -> FillHist1Step("plane_fail",rms,200,0,200);
#endif
    return kTRUE;
  }
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1("plane",rms,200,0,20);
#endif

  survive = kTRUE;
  return kTRUE;
}
