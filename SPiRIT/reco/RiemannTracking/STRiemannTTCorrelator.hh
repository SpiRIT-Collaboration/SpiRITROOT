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

#ifndef STRIEMANNTTCORRELATOR_HH
#define STRIEMANNTTCORRELATOR_HH

// SpiRITROOT classes
#include "STAbsTrackTrackCorrelator.hh"

class STRiemannTTCorrelator : public STAbsTrackTrackCorrelator
{
  public:
    STRiemannTTCorrelator(Double_t planecut, Int_t minHitsForFit);

    virtual Bool_t Correlate(STRiemannTrack *track1, STRiemannTrack *track2, Bool_t &survive, Double_t &matchQuality);

  private:
    Double_t fPlaneCut;
    Int_t fMinHitsForFit;
};

#endif
