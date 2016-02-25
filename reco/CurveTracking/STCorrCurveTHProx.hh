#ifndef STCTPROXIMITYHTCORRELATOR
#define STCTPROXIMITYHTCORRELATOR

#include "STCurveTrackingConf.hh"

class STCorrCurveTHProx : public STCorrCurveTH
{
  public:
    STCorrCurveTHProx(Double_t xCut, 
                       Double_t yCut, 
                       Double_t zCut,
                          Int_t nHitsCompare,
                          Int_t nHitsCompareMax);

    Bool_t Correlate(STCurveTrack *track, 
                     STHit *hit, 
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Double_t fXProxCut;
    Double_t fYProxCut;
    Double_t fZProxCut;

    Int_t fNumHitsCompare;
    Int_t fNumHitsCompareMax;
};

#endif
