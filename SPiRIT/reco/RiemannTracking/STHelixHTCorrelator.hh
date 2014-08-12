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

#ifndef STHELIXHTCORRELATOR_HH
#define STHELIXHTCORRELATOR_HH

// SpiRITROOT classes
#include "STAbsHitTrackCorrelator.hh"

class STHelixHTCorrelator : public STAbsHitTrackCorrelator
{
  public:
    STHelixHTCorrelator(Double_t hdist);

    virtual Bool_t Correlate(STRiemannTrack* track, STRiemannHit* rhit, Bool_t& survive, Double_t& matchQuality);

  private:
    Double_t fHDistCut;
};

#endif
