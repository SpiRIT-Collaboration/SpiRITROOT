#ifndef STCORRLINEARTTGEO
#define STCORRLINEARTTGEO

#include "STLinearTrackingConf.hh"

class STCorrLinearTTGeo : public STCorrLinearTT
{
  public:
    STCorrLinearTTGeo(Int_t nHitsToFitTrack,
                      Double_t dotDrectionCut,
                      Double_t dotNormalCut);

    Bool_t Correlate(STLinearTrack *track1, 
                     STLinearTrack *track2,
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Int_t fMinNumHitsToFitTrack;
    Double_t fDotDirectionCut;
    Double_t fDotNormalCut;
};

#endif
