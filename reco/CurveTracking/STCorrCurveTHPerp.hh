#ifndef STCORRCURVETHPERP
#define STCORRCURVETHPERP

#include "STCurveTrackingConf.hh"

class STCorrCurveTHPerp : public STCorrCurveTH
{
  public:
    STCorrCurveTHPerp(Int_t nHitsToFitTrack,
                       Double_t perpLineCut,
                       Double_t perpPlaneCut);

    Bool_t Correlate(STCurveTrack *track, 
                     STHit *hit, 
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Int_t fMinNumHitsToFitTrack;
    Double_t fPerpLineCut;
    Double_t fPerpPlaneCut;
};

#endif
