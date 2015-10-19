#ifndef STCORRLINEARTHPERP
#define STCORRLINEARTHPERP

#include "STLinearTrackingConf.hh"

class STCorrLinearTHPerp : public STCorrLinearTH
{
  public:
    STCorrLinearTHPerp(Int_t nHitsToFitTrack,
                       Double_t perpLineCut,
                       Double_t perpPlaneCut);

    Bool_t Correlate(STLinearTrack *track, 
                     STHit *hit, 
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Int_t fMinNumHitsToFitTrack;
    Double_t fPerpLineCut;
    Double_t fPerpPlaneCut;
};

#endif
