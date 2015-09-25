/**
 * @brief Hit-Track-Correlator using Helix fit
 *
 * @author Johannes Rauch (TUM) -- original author
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 */

// SpiRITROOT classes
#include "STHit.hh"
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"
#include "STHelixHTCorrelator.hh"
#include "STDebugLogger.hh"

// ROOT classes
#include "TVector3.h"
#include "TMath.h"

#include "Riostream.h"

STHelixHTCorrelator::STHelixHTCorrelator(Double_t hdistcut)
{
  fHDistCut = hdistcut;
}

Bool_t
STHelixHTCorrelator::Correlate(STRiemannTrack *track, STRiemannHit *rhit, Bool_t &survive, Double_t &matchQuality)
{
  if (!(track -> IsFitted()))
    return kFALSE;

  Double_t distHelix = track -> DistHelix(rhit);
  distHelix = TMath::Abs(distHelix);
  matchQuality = distHelix;

  Double_t stat = 2. - track -> GetQuality(); // tighten cut for better defined tracks

  // for best track:  fHDistCut < distHelix, 
  // for worst track: 2*fHDistCut < distHelix, then die
  if (distHelix > fHDistCut*stat) 
  {
    survive = kFALSE;
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1Step("helix_fail", distHelix, 1000, 0, 1000);
#endif
    return kTRUE;
  }
#ifdef DEBUGRIEMANNCUTS
  STDebugLogger::Instance() -> FillHist1("helix", distHelix, 100, 0, 20);
#endif

  survive = kTRUE;
  return kTRUE;
}

