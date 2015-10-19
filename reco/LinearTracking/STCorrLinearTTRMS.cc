#include "STCorrLinearTTRMS.hh"

STCorrLinearTTRMS::STCorrLinearTTRMS(Int_t nHitsToFitCut,
                                       Double_t rmsLine,
                                       Double_t rmsPlane)
{
  nHitsToFitCut = fMinNumHitsToFitTrack;
  fRMSLineCut = rmsLine;
  fRMSPlaneCut = rmsPlane;
}

Bool_t 
STCorrLinearTTRMS::Correlate(STLinearTrack *track1,
                             STLinearTrack *track2,
                             Bool_t &survive, 
                             Double_t &matchQuality)
{
  survive = kFALSE;
  Double_t scaling = 2 - matchQuality;
  Double_t rmsLineCut  = scaling * fRMSLineCut;
  Double_t rmsPlaneCut = scaling * fRMSPlaneCut;

  if (track1 -> GetNumHits() + track2 -> GetNumHits() <= fMinNumHitsToFitTrack)
    return kFALSE;

  if (track1 -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track1);

  if (track2 -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track2);

  Double_t rmsLine, rmsPlane;  
  fLTFitter -> Fit(track1, track2, rmsLine, rmsPlane);

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
