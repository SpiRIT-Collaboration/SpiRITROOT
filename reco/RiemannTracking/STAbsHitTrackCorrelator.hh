#ifndef STABSHITTRACKCORRELATOR_HH
#define STABSHITTRACKCORRELATOR_HH

// ROOT classes
#include "Rtypes.h"

// SpiRITROOT classes
class STRiemannTrack;
class STRiemannHit;

/**
 * Abstract interface for a hit-track correlator
 * for the Riemann track finder
 *
 * @author Johannes Rauch (TUM) -- original author
 * @author Genie Jhang    (Korea University)
 * @author JungWoo Lee (Korea University)
 */

class STAbsHitTrackCorrelator 
{
  public:
    STAbsHitTrackCorrelator(){}
    virtual ~STAbsHitTrackCorrelator(){}

    /**
      * Correlation method:
      *   Returns true if the correlator is applicable to this track/hit combo.
      *   Survival is set to true if track survives the correlator.
      *   In this case matchQuality is a measure of how well the hit lies
      *   on the track with restpect to this correlator.
     **/
    virtual Bool_t Correlate(STRiemannTrack *track, 
                             STRiemannHit *hit, 
                             Bool_t &survive, 
                             Double_t &matchQuality) = 0;
};

#endif
