// SpiRITROOT classes
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"
#include "STStraightLineCorrelator.hh"

// ROOT classes

#include "STDebugLogger.hh"

STStraightLineCorrelator::STStraightLineCorrelator(Double_t hdistcut)
{
}

Bool_t
STStraightLineCorrelator::Correlate(STRiemannTrack *track, STRiemannHit *rhit, Bool_t &survive, Double_t &matchQuality)
{
  if (!(track -> IsFitted()))
    return kFALSE;

  Double_t d = track -> DistHelix(rhit);
  d = TMath::Abs(d);
  matchQuality = d;

  Double_t stat = 2. - track -> GetQuality(); // tighten cut for better defined tracks

  if (d > fHDistCut*stat)
  {
    survive = kFALSE;
    return kTRUE;
  }

  survive = kTRUE;
  return kTRUE;
}

