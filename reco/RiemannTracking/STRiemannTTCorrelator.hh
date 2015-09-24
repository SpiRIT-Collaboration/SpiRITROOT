/**
 * @brief Track-Track-Correlator using riemann fit
 *
 * @author Sebastian Neubert (TUM) -- original author
 * @author Johannes Rauch    (TUM)
 * @author Genie Jhang (Korea University)
 */

#ifndef STRIEMANNTTCORRELATOR_HH
#define STRIEMANNTTCORRELATOR_HH

// SpiRITROOT classes
#include "STAbsTrackTrackCorrelator.hh"

class STRiemannTTCorrelator : public STAbsTrackTrackCorrelator
{
  public:
    STRiemannTTCorrelator(Double_t planecut, Int_t minHitsForFit);

    virtual Bool_t Correlate(STRiemannTrack *track1, 
                             STRiemannTrack *track2, 
                             Bool_t &survive, 
                             Double_t &matchQuality);

  private:
    Double_t fPlaneCut;
    Int_t fMinHitsForFit;
};

#endif
