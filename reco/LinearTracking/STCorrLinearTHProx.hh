#ifndef STLTPROXIMITYHTCORRELATOR
#define STLTPROXIMITYHTCORRELATOR

#include "STLinearTrackingConf.hh"

class STCorrLinearTHProx : public STCorrLinearTH
{
  public:
    STCorrLinearTHProx(Int_t nHitsCompare,
                       Double_t xCut, 
                       Double_t yCut, 
                       Double_t zCut);

    Bool_t Correlate(STLinearTrack *track, 
                     STHit *hit, 
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Double_t fXProxCut;
    Double_t fYProxCut;
    Double_t fZProxCut;

    Int_t fNumHitsCompare;
};

#endif
