#ifndef STCORRLINEARTTRMS
#define STCORRLINEARTTRMS

#include "STLinearTrackingConf.hh"

class STCorrLinearTTRMS : public STCorrLinearTT
{
  public:
    STCorrLinearTTRMS(Int_t nHitsToFitCut,
                      Double_t rmsLine,
                      Double_t rmsPlane);

    Bool_t Correlate(STLinearTrack *track1,
                     STLinearTrack *track2,
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Int_t fMinNumHitsToFitTrack;
    Double_t fRMSLineCut;
    Double_t fRMSPlaneCut;
};

#endif
