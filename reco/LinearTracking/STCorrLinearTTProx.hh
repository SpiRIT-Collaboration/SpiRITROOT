#ifndef STCORRLINEARTTPROX
#define STCORRLINEARTTPROX

#include "STLinearTrackingConf.hh"

class STCorrLinearTTProx : public STCorrLinearTT
{
  public:
    STCorrLinearTTProx(Int_t minNumHitsCut, Double_t proxCut) 
    : fMinNumHitsCut(minNumHitsCut), fProxCut(proxCut) {}

    Bool_t Correlate(STLinearTrack *track1, 
                     STLinearTrack *track2,
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Int_t fMinNumHitsCut;
    Double_t fProxCut;
};

#endif
