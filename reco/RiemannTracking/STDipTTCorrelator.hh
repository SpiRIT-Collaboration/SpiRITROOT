/**
 * @brief Track-Track-Correlator using dip fit
 *
 * @author Sebastian Neubert (TUM) -- original author
 * @author Johannes Rauch    (TUM)
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 */

#ifndef STPDIPTTCORRELATOR_HH
#define STPDIPTTCORRELATOR_HH

// SpiRITROOT classes
#include "STAbsTrackTrackCorrelator.hh"

class STDipTTCorrelator : public STAbsTrackTrackCorrelator
{
  public:
    STDipTTCorrelator(Double_t proxcut, Double_t dipcut, Double_t helixcut);

    virtual Bool_t Correlate(STRiemannTrack* track1, 
                             STRiemannTrack* track2, 
                             Bool_t& survive, 
                             Double_t& matchQuality);

  private:
    Double_t fProxCut;
    Double_t fDipCut;
    Double_t fHelixCut;
};

#endif
