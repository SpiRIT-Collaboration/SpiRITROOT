#ifndef STSTRAIGHTLINECORRELATOR_HH
#define STSTRAIGHTLINECORRELATOR_HH

// SpiRITROOT classes
#include "STAbsHitTrackCorrelator.hh"

class STStraightLineCorrelator : public STAbsHitTrackCorrelator
{
  public:
    STStraightLineCorrelator(Double_t hdist);

    virtual Bool_t Correlate(STRiemannTrack* track, STRiemannHit* rhit, Bool_t& survive, Double_t& matchQuality);

  private:
};

#endif
