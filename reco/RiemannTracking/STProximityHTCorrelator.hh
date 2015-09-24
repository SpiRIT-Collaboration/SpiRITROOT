/**
 * @brief Hit-Track-Correlator using proximity arguments* 
 *
 * @atrhor Sebastian Neubert (TUM) -- original author
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 */

#ifndef STPROXIMITYHTCORRELATOR_HH
#define STPROXIMITYHTCORRELATOR_HH

// SpiRITROOT classes
#include "STAbsHitTrackCorrelator.hh"

class STProximityHTCorrelator : public STAbsHitTrackCorrelator
{
  public:
    STProximityHTCorrelator(Double_t cut, Double_t zStretch = 1., Double_t helixcut = 1.0);

    virtual Bool_t Correlate(STRiemannTrack *trk, 
                             STRiemannHit *rhit, 
                             Bool_t &survive, 
                             Double_t &matchQuality);

  private:
    Double_t fProxCut;
    Double_t fZStretch;
    Double_t fMeanDist; ///< mean distance between hits in track
    Double_t fHelixCut; ///< for fast approximation
};

#endif
