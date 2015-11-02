#include "STCorrLinearTTPerp.hh"

STCorrLinearTTPerp::STCorrLinearTTPerp(Int_t nHitsToFitTrack,
                                       Double_t perpLineCut,
                                       Double_t perpPlaneCut)
{
  fMinNumHitsToFitTrack = nHitsToFitTrack;
  fPerpLineCut = perpLineCut;
  fPerpPlaneCut = perpPlaneCut;
}

Bool_t 
STCorrLinearTTPerp::Correlate(STLinearTrack *track1, 
                              STLinearTrack *track2,
                              Bool_t &survive, 
                              Double_t &matchQuality)
{
  survive = kFALSE;
  Int_t nHitsTrack1 = track1 -> GetNumHits();
  Int_t nHitsTrack2 = track2 -> GetNumHits();

  if (nHitsTrack1 <= fMinNumHitsToFitTrack || nHitsTrack2 <= fMinNumHitsToFitTrack)
    return kFALSE;

  Double_t scaling = 2 - matchQuality;
  Double_t perpLineCut   = scaling * fPerpLineCut;
  Double_t perpPlaneCut = scaling * fPerpPlaneCut;

  if (track1 -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track1);

  if (track2 -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track2);

  STLinearTrack *trackLong;
  STLinearTrack *trackShort;

  if (nHitsTrack1 > nHitsTrack2) {
    trackLong = track1;
    trackShort = track2;
  }
  else {
    trackLong = track2;
    trackShort = track1;
  }

  STHit* hitFirst = trackShort -> GetHit(0);
  TVector3 perpLineFirst  = fLTFitter -> PerpLine (trackLong, hitFirst);
  TVector3 perpPlaneFirst = fLTFitter -> PerpPlane(trackLong, hitFirst);
           perpLineFirst  = perpLineFirst - perpPlaneFirst;

  Double_t distLineFirst  = perpLineFirst.Mag();
  Double_t distPlaneFirst = perpPlaneFirst.Mag();


  STHit* hitLast = trackShort -> GetHit(trackShort -> GetNumHits() - 1);
  TVector3 perpLineLast  = fLTFitter -> PerpLine (trackLong, hitLast);
  TVector3 perpPlaneLast = fLTFitter -> PerpPlane(trackLong, hitLast);
           perpLineLast  = perpLineLast - perpPlaneLast;

  Double_t distLineLast  = perpLineLast.Mag();
  Double_t distPlaneLast = perpPlaneLast.Mag();


  TVector3 hitCentroid = trackShort -> GetCentroid();
  TVector3 perpLineCentroid  = fLTFitter -> PerpLine (trackLong, hitCentroid);
  TVector3 perpPlaneCentroid = fLTFitter -> PerpPlane(trackLong, hitCentroid);
           perpLineCentroid  = perpLineCentroid - perpPlaneCentroid;

  Double_t distLineCentroid  = perpLineCentroid.Mag();
  Double_t distPlaneCentroid = perpPlaneCentroid.Mag();


  Double_t distL = distLineFirst < distLineLast ? distLineFirst : distLineLast;
  Double_t distP = distPlaneFirst < distPlaneLast ? distPlaneFirst : distPlaneLast;

           distL = distL < distLineCentroid ? distL : distLineCentroid;
           distP = distL < distPlaneCentroid ? distP : distPlaneCentroid;

  Double_t corrLine  = (perpLineCut  - distL) / perpLineCut;
  Double_t corrPlane = (perpPlaneCut - distP) / perpPlaneCut;

  if (corrLine >= 0 && corrPlane >= 0) 
  {
    matchQuality = TMath::Sqrt(matchQuality*matchQuality + 
                               corrLine*corrLine + 
                               corrPlane*corrPlane)
                   / (corrLine + corrPlane + matchQuality);

    survive = kTRUE;
  }

  return kTRUE;
}
