#include "STCorrLinearTHRMS.hh"

STCorrLinearTHRMS::STCorrLinearTHRMS(Int_t nHitsToFitCut,
                                     Double_t rmsLine,
                                     Double_t rmsPlane)
{
  fMinNumHitsToFitTrack = nHitsToFitCut;
  fRMSLineCut = rmsLine;
  fRMSPlaneCut = rmsPlane;
}

Bool_t 
STCorrLinearTHRMS::Correlate(STLinearTrack *track, 
                                STHit *hit, 
                                Bool_t &survive, 
                                Double_t &matchQuality)
{
  survive = kTRUE;

  if (track -> GetNumHits() <= fMinNumHitsToFitTrack)
    return kFALSE;

  survive = kFALSE;

  Double_t scaling = 2 - matchQuality;
  Double_t rmsLineCut  = scaling * fRMSLineCut;
  Double_t rmsPlaneCut = scaling * fRMSPlaneCut;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t rmsLine, rmsPlane;  
  fLTFitter -> Fit(track, hit, rmsLine, rmsPlane);

#ifdef DEBUGLIT_CUT
  STDebugLogger::Instance() -> FillHist1("rmsL", rmsLine,  100, 0, 30);
  STDebugLogger::Instance() -> FillHist1("rmsP", rmsPlane, 100, 0, 10);
#endif

  Double_t corrLine  = (rmsLineCut  - rmsLine)  / rmsLineCut;
  Double_t corrPlane = (rmsPlaneCut - rmsPlane) / rmsPlaneCut;

  if (corrLine >= 0 && corrPlane >= 0) 
  {
    matchQuality = TMath::Sqrt(matchQuality*matchQuality + corrLine*corrLine + corrPlane*corrPlane)
                   / (corrLine + corrPlane + matchQuality);
    survive = kTRUE;
  }

  return kTRUE;
}
