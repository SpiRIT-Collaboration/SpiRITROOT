//-----------------------------------------------------------
// Description:
//      Track-Track-Correlator using riemann fit
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
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"
#include "STRiemannTTCorrelator.hh"

// ROOT classes
#include "TVector3.h"
#include "TMath.h"

//#include "DebugLogger.h"

STRiemannTTCorrelator::STRiemannTTCorrelator(Double_t planecut, Int_t minHitsForFit)
{
  fPlaneCut = planecut;
  fMinHitsForFit = minHitsForFit;
}

Bool_t
STRiemannTTCorrelator::Correlate(STRiemannTrack *track1, STRiemannTrack *track2, Bool_t &survive, Double_t &matchQuality)
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
//  DebugLogger::Instance() -> Histo("TT_riem_rms",rms,0,0.005,100);

  if (rms > fPlaneCut*scaling) {
//    DebugLogger::Instance() -> Histo("TT_riemanncuts",6,0,20,20);
    survive = kFALSE;
    return kTRUE;
  }

  survive = kTRUE;
  return kTRUE;
}
