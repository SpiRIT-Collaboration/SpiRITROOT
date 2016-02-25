#include "STCorrCurveTHPerp.hh"

#include <iostream>
using namespace std;

STCorrCurveTHPerp::STCorrCurveTHPerp(Int_t nHitsToFitTrack,
                                       Double_t perpLineCut,
                                       Double_t perpPlaneCut)
{
  fMinNumHitsToFitTrack = nHitsToFitTrack;
  fPerpLineCut = perpLineCut;
  fPerpPlaneCut = perpPlaneCut;
}

Bool_t 
STCorrCurveTHPerp::Correlate(STCurveTrack *track, 
                             STHit *hit, 
                             Bool_t &survive, 
                             Double_t &matchQuality)
{
  survive = kTRUE;

  if (track -> GetNumHits() - track -> GetNumHitsRemoved() <= fMinNumHitsToFitTrack){
#ifdef DEBUG_CURVE
    cout << "In perp corr, not enough number of hits to fit!" << endl;
#endif
    return kFALSE;
  }

  survive = kFALSE;

  Double_t scaling = 2 - matchQuality;
  Double_t perpLineCut  = scaling * track -> GetRMSLine() * 4;//scaling * fPerpLineCut;
  Double_t perpPlaneCut = scaling * track -> GetRMSPlane() * 3;

  if (perpLineCut < fPerpLineCut) perpLineCut = fPerpLineCut;
  if (perpPlaneCut < fPerpPlaneCut) perpPlaneCut = fPerpPlaneCut;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  TVector3 perpLine  = fLTFitter -> PerpLine(track, hit);
  TVector3 perpPlane = fLTFitter -> PerpPlane(track, hit);
           perpLine  = perpLine - perpPlane;

  Double_t distLine  = perpLine.Mag();
  Double_t distPlane = perpPlane.Mag();

#ifdef DEBUG_CURVE
  Double_t b = track -> GetNumHits() - track -> GetNumHitsRemoved();
  Double_t a = track -> GetChargeSum() / b;
  Double_t distLineAfter = sqrt(hit -> GetCharge() * distLine * distLine / a);
    cout << "perp:  " 
         << distLineAfter << " <l? " << perpLineCut  << " | "
         << distPlane     << " <p? " << perpPlaneCut << "  (" 
         << hit -> GetCharge()      << ", " 
         << distLine                << ", " 
         << b                       << ", " 
         << track -> GetChargeSum() << ", " 
         << a << ")" << endl;
#endif

  distLine = sqrt(hit -> GetCharge() * distLine * distLine * (track -> GetNumHits() - track -> GetNumHitsRemoved()) / track -> GetChargeSum());

#ifdef DEBUGLIT_CUT
  STDebugLogger::Instance() -> FillHist1("dL", distLine,  100, 0, 30);
  STDebugLogger::Instance() -> FillHist1("dP", distPlane, 100, 0, 10);
#endif

  Double_t corrLine  = (perpLineCut  - distLine)  / perpLineCut;
  Double_t corrPlane = (perpPlaneCut - distPlane) / perpPlaneCut;

  if (corrLine >= 0 && corrPlane >= 0) 
  {
    survive = kTRUE;
    matchQuality = sqrt((corrLine*corrLine + corrPlane*corrPlane + matchQuality*matchQuality) / 3);
#ifdef DEBUG_CURVE
    cout << "quality: " << matchQuality << endl;
#endif
  }

  return kTRUE;
}
