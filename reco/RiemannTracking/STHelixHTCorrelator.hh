/**
 * @brief Hit-Track-Correlator using Helix fit
 *
 * @author Johannes Rauch (TUM) -- original author
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 */

#ifndef STHELIXHTCORRELATOR_HH
#define STHELIXHTCORRELATOR_HH

// SpiRITROOT classes
#include "STAbsHitTrackCorrelator.hh"

class STHelixHTCorrelator : public STAbsHitTrackCorrelator
{
  public:
    STHelixHTCorrelator(Double_t hdist);

    virtual Bool_t Correlate(STRiemannTrack* track, 
                             STRiemannHit* rhit, 
                             Bool_t& survive, 
                             Double_t& matchQuality);

  private:
    Double_t fHDistCut;
};

#endif
