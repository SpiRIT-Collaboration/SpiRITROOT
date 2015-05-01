//-----------------------------------------------------------
// Description:
//      Track-Track-Correlator using dip fit
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

#ifndef STPDIPTTCORRELATOR_HH
#define STPDIPTTCORRELATOR_HH

// SpiRITROOT classes
#include "STAbsTrackTrackCorrelator.hh"

class STDipTTCorrelator : public STAbsTrackTrackCorrelator
{
  public:
    STDipTTCorrelator(Double_t proxcut, Double_t dipcut, Double_t helixcut);

    virtual Bool_t Correlate(STRiemannTrack* track1, STRiemannTrack* track2, Bool_t& survive, Double_t& matchQuality);

  private:
    Double_t fProxCut;
    Double_t fDipCut;
    Double_t fHelixCut;
};

#endif
