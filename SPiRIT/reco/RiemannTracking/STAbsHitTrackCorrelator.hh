//-----------------------------------------------------------
// Description:
//      Abstract interface for a hit-track correlator
//      for the Riemann track finder
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Original Author:
//      Sebastian Neubert    TUM            (original author)
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------

#ifndef STABSHITTRACKCORRELATOR_HH
#define STABSHITTRACKCORRELATOR_HH

// ROOT classes
#include "Rtypes.h"

// SpiRITROOT classes
class STRiemannTrack;
class STRiemannHit;

class STAbsHitTrackCorrelator {
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
    virtual Bool_t Correlate(STRiemannTrack *track, STRiemannHit *hit, Bool_t &survive, Double_t &matchQuality) = 0;
};

#endif
