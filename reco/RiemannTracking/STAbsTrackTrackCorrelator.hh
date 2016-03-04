#ifndef STABSTRACKTRACKCORRELATOR_HH
#define STABSTRACKTRACKCORRELATOR_HH

// ROOT classes
#include "Rtypes.h"
#include "STGlobal.hh"

// SpiRITROOT classes
class STRiemannTrack;
class STRiemannHit;

/**
 * Abstract interface for a track-track correlator
 * for the Riemann track finder
 *
 * @author Johannes Rauch (TUM) -- original author
 * @author Genie Jhang    (Korea University)
 * @author JungWoo Lee (Korea University)
 */

class STAbsTrackTrackCorrelator 
{
  public:
    STAbsTrackTrackCorrelator(){}
    virtual ~STAbsTrackTrackCorrelator(){}

    /**
      * Correlation method:
      *   Returns true if correlator is applicable to this track/track combo.
      *   Survival is set to true if the track survives the correlator.
      *   In this case, matchQuality is a measure of how well the tracks correlated.
     **/
    virtual Bool_t Correlate(STRiemannTrack *track1, 
                             STRiemannTrack *track2, 
                             Bool_t &survive, 
                             Double_t &matchQuality) = 0;

};

#endif
