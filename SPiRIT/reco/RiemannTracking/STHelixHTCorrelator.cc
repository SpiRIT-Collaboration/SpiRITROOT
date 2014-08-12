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

//#include "DebugLogger.h"

STHelixHTCorrelator::STHelixHTCorrelator(Double_t hdistcut)
{
  fHDistCut = hdistcut;
}


Bool_t
STHelixHTCorrelator::Correlate(STRiemannTrack *track, STRiemannHit *rhit, Bool_t &survive, Double_t &matchQuality)
{
  // check if we have a fit:
  if(!(track -> IsFitted()))
    return kFALSE;

  Double_t d = track -> DistHelix(rhit);
  //std::cout<<"distance to plane: "<< d<<std::endl;
//  DebugLogger::Instance() -> Histo("HT_helix_dist",d,0,0.2,100);
  d = TMath::Abs(d);
  matchQuality = d;

  Double_t stat = 2. - track -> GetQuality(); // tighten cut for better defined tracks

  if ( d > fHDistCut*stat) {
//    DebugLogger::Instance() -> Histo("HT_riemanncuts",8,0,20,20);
    survive = kFALSE;
    return kTRUE;
  }

  survive = kTRUE;
  return kTRUE;
}

