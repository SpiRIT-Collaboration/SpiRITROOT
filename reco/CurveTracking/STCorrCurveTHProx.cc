#include "STCorrCurveTHProx.hh"

#include <iostream>
using namespace std;

STCorrCurveTHProx::STCorrCurveTHProx(
  Double_t xProxCut,
  Double_t yProxCut,
  Double_t zProxCut,
     Int_t nHitsCompare,
     Int_t nHitsCompareMax)
{
  fXProxCut = xProxCut;
  fYProxCut = yProxCut;
  fZProxCut = zProxCut;

  fNumHitsCompare = nHitsCompare;
  fNumHitsCompareMax = nHitsCompareMax;
}

Bool_t 
STCorrCurveTHProx::Correlate(STCurveTrack *track, 
                             STHit *hit, 
                             Bool_t &survive, 
                             Double_t &matchQuality)
{
  survive = kTRUE;

  Int_t numHitsInTrack = track -> GetNumHits();
  Int_t numHitsCompare = numHitsInTrack - track -> GetNumHitsRemoved();

  survive = kFALSE;

  if (numHitsCompare > fNumHitsCompare) {
    numHitsCompare = fNumHitsCompare;
  }

  Double_t xCut = fXProxCut;
  Double_t yCut = fYProxCut;
  Double_t zCut = fZProxCut;

  TVector3 positionHit = hit -> GetPosition();

  STHit *hitT = track -> GetHit(numHitsInTrack - 1);
  TVector3 positionHitInTrack = hitT -> GetPosition();

  Double_t dX = positionHitInTrack.X() - positionHit.X();
  Double_t dY = positionHitInTrack.Y() - positionHit.Y();
  Double_t dZ = positionHitInTrack.Z() - positionHit.Z();

  if (dX < 0) dX *= -1;
  if (dY < 0) dY *= -1;
  if (dZ < 0) dZ *= -1;

#ifdef DEBUG_CURVE
  cout << "ppre:  "
       << dX << " <? " << xCut * 5 << " | "
       << dY << " <? " << yCut * 5 << " | "
       << dZ << " <? " << zCut * 5 << endl;
#endif

  if ((dX < xCut * 5 && dY < yCut * 5 && dZ < zCut * 5) == kFALSE)
  {
    survive = kFALSE;
    return kTRUE;
  }

  for (Int_t iHit = 0; iHit < numHitsCompare; iHit++)
  {
    hitT = track -> GetHit(numHitsInTrack - 1 - iHit);
    positionHitInTrack = hitT -> GetPosition();

    dX = positionHitInTrack.X() - positionHit.X();
    dY = positionHitInTrack.Y() - positionHit.Y();
    dZ = positionHitInTrack.Z() - positionHit.Z();

#ifdef DEBUGLIT_CUT
    STDebugLogger::Instance() -> FillHist1("dx", dX, 100, 0, 30);
    STDebugLogger::Instance() -> FillHist1("dy", dY, 100, 0, 30);
    STDebugLogger::Instance() -> FillHist1("dz", dZ, 100, 0, 10);
#endif

    if (dX < 0) dX *= -1;
    if (dY < 0) dY *= -1;
    if (dZ < 0) dZ *= -1;

#ifdef DEBUG_CURVE
    cout << "prox:  "
         << dX << " <? " << xCut << " | "
         << dY << " <? " << yCut << " | "
         << dZ << " <? " << zCut << endl;
#endif

    Double_t corrX = (xCut - dX) / xCut;
    Double_t corrY = (yCut - dY) / yCut;
    Double_t corrZ = (zCut - dZ) / zCut;

    if (corrX >= 0 && corrY >= 0 && corrZ >= 0) 
    {
      survive = kTRUE;
      matchQuality = sqrt((corrX*corrX + corrY*corrY + corrZ*corrZ + matchQuality*matchQuality) / 4);
#ifdef DEBUG_CURVE
      cout << "quality: " << matchQuality << endl;
#endif
      return kTRUE;
    }
  }

  return kTRUE;
}
