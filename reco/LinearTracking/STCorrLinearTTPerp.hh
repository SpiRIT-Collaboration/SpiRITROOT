#ifndef STCORRLINEARTTPERP
#define STCORRLINEARTTPERP

#include "STLinearTrackingConf.hh"

class STCorrLinearTTPerp : public STCorrLinearTT
{
  public:
    STCorrLinearTTPerp(Int_t nHitsToFitTrack,
                       Double_t perpLineCut,
                       Double_t perpPlaneCut);

    Bool_t Correlate(STLinearTrack *track1, 
                     STLinearTrack *track2,
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Int_t fMinNumHitsToFitTrack;
    Double_t fPerpLineCut;
    Double_t fPerpPlaneCut;
};

#endif
