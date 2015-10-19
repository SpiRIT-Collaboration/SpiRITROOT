#ifndef STCORRLINEARTHRMS
#define STCORRLINEARTHRMS

#include "STLinearTrackingConf.hh"

class STCorrLinearTHRMS : public STCorrLinearTH
{
  public:
    STCorrLinearTHRMS(Int_t nHitsToFitCut,
                       Double_t rmsLine,
                       Double_t rmsPlane);

    Bool_t Correlate(STLinearTrack *track, 
                     STHit *hit, 
                     Bool_t &survive, 
                     Double_t &matchQuality);

  private:
    Int_t fMinNumHitsToFitTrack;
    Double_t fRMSLineCut;
    Double_t fRMSPlaneCut;
};

#endif
