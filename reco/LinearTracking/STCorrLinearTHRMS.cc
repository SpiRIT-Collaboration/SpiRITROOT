#include "STCorrLinearTHRMS.hh"

STCorrLinearTHRMS::STCorrLinearTHRMS(Int_t nHitsToFitCut,
                                       Double_t rmsLine,
                                       Double_t rmsPlane)
{
  nHitsToFitCut = fMinNumHitsToFitTrack;
  fRMSLineCut = rmsLine;
  fRMSPlaneCut = rmsPlane;
}

Bool_t 
STCorrLinearTHRMS::Correlate(STLinearTrack *track, 
                                STHit *hit, 
                                Bool_t &survive, 
                                Double_t &matchQuality)
{
  survive = kFALSE;
  Double_t scaling = 2 - matchQuality;
  Double_t rmsLineCut  = scaling * fRMSLineCut;
  Double_t rmsPlaneCut = scaling * fRMSPlaneCut;

  if (track -> GetNumHits() <= fMinNumHitsToFitTrack)
    return kFALSE;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t rmsLine, rmsPlane;  
  fLTFitter -> Fit(track, hit, rmsLine, rmsPlane);

  Double_t corrLine  = (rmsLineCut  - rmsLine)  / rmsLineCut;
  Double_t corrPlane = (rmsPlaneCut - rmsPlane) / rmsPlaneCut;

  if (corrLine <= 0 && corrPlane <= 0) 
  {
    matchQuality = TMath::Sqrt(matchQuality*matchQuality + 
                               corrLine*corrLine + 
                               corrPlane*corrPlane);
    survive = kTRUE;
  }

  return kTRUE;
}
