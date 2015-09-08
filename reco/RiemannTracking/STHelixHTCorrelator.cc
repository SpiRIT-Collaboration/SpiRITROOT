//-----------------------------------------------------------
// Description:
//      Hit-Track-Correlator using Helix fit
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Original Author List:
//      Johanness Rauch    TUM            (original author)
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STHitCluster.hh"
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"
#include "STHelixHTCorrelator.hh"

// ROOT classes
#include "TVector3.h"
#include "TMath.h"

#include "Riostream.h"

#include "STDebugLogger.hh"

STHelixHTCorrelator::STHelixHTCorrelator(Double_t hdistcut)
{
  fHDistCut = hdistcut;
}

Bool_t
STHelixHTCorrelator::Correlate(STRiemannTrack *track, STRiemannHit *rhit, Bool_t &survive, Double_t &matchQuality)
{
  if (!(track -> IsFitted()))
    return kFALSE;

  Double_t d = track -> DistHelix(rhit);
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1Step("helix_all",d,1000,0,1000);
#endif
  d = TMath::Abs(d);
  matchQuality = d;

  Double_t stat = 2. - track -> GetQuality(); // tighten cut for better defined tracks

  if (d > fHDistCut*stat) // for best track: fHDistCut < d, for worst track: 2*fHDistCut < d, then die
  {
    survive = kFALSE;
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1Step("helix_fail",d,1000,0,1000);
#endif
    return kTRUE;
  }
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1("helix",d,100,0,200);
  STDebugLogger::Instance() -> FillHist1Step("helix_step",d,1000,0,1000);
#endif

  survive = kTRUE;
  return kTRUE;
}

