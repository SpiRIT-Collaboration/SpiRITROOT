#include "STCorrLinearTHPerp.hh"

STCorrLinearTHPerp::STCorrLinearTHPerp(Int_t nHitsToFitTrack,
                                       Double_t perpLineCut,
                                       Double_t perpPlaneCut)
{
  fMinNumHitsToFitTrack = nHitsToFitTrack;
  fPerpLineCut = perpLineCut;
  fPerpPlaneCut = perpPlaneCut;
}

Bool_t 
STCorrLinearTHPerp::Correlate(STLinearTrack *track, 
                              STHit *hit, 
                              Bool_t &survive, 
                              Double_t &matchQuality)
{
  survive = kTRUE;

  if (track -> GetNumHits() <= fMinNumHitsToFitTrack)
    return kFALSE;

  survive = kFALSE;

  Double_t scaling = 2 - matchQuality;
  Double_t perpLineCut  = scaling * fPerpLineCut;
  Double_t perpPlaneCut = scaling * fPerpPlaneCut;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  TVector3 perpLine  = fLTFitter -> PerpLine(track, hit);
  TVector3 perpPlane = fLTFitter -> PerpPlane(track, hit);
           perpLine  = perpLine - perpPlane;

  Double_t distLine  = perpLine.Mag();
  Double_t distPlane = perpPlane.Mag();

#ifdef DEBUGLIT_CUT
    STDebugLogger::Instance() -> FillHist1("dL", distLine,  100, 0, 30);
    STDebugLogger::Instance() -> FillHist1("dP", distPlane, 100, 0, 10);
#endif

  Double_t corrLine  = (perpLineCut  - distLine)  / perpLineCut;
  Double_t corrPlane = (perpPlaneCut - distPlane) / perpPlaneCut;

  if (corrLine >= 0 && corrPlane >= 0) 
  {
    survive = kTRUE;
    matchQuality = TMath::Sqrt(matchQuality*matchQuality + corrLine*corrLine + corrPlane*corrPlane)
                   / (corrLine + corrPlane + matchQuality);
  }

  return kTRUE;
}
