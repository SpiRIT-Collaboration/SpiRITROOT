#include "STCorrLinearTHProx.hh"

#include <iostream>

using namespace std;

STCorrLinearTHProx::STCorrLinearTHProx(
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
STCorrLinearTHProx::Correlate(STLinearTrack *track, 
                              STHit *hit, 
                              Bool_t &survive, 
                              Double_t &matchQuality)
{
  survive = kTRUE;

  Int_t numHitsInTrack = track -> GetNumHits();
  Int_t numHitsCompare = numHitsInTrack;

  if (numHitsCompare >= fNumHitsCompareMax)
    return kFALSE;

  survive = kFALSE;
  Double_t scaling = 2;

  if (numHitsCompare > fNumHitsCompare) {
    numHitsCompare = fNumHitsCompare;
    scaling = 1;
  }

  Double_t xCut = scaling * fXProxCut;
  Double_t yCut =           fYProxCut; // no scaling for y
  Double_t zCut = scaling * fZProxCut;

  TVector3 positionHit = hit -> GetPosition();

  for (Int_t iHit = 0; iHit < numHitsCompare; iHit++)
  {
    STHit *hitT = track -> GetHit(numHitsInTrack - 1 - iHit);
    TVector3 positionHitInTrack = hitT -> GetPosition();

    Double_t dX = positionHitInTrack.X() - positionHit.X();
    Double_t dY = positionHitInTrack.Y() - positionHit.Y();
    Double_t dZ = positionHitInTrack.Z() - positionHit.Z();

#ifdef DEBUGLIT_CUT
    STDebugLogger::Instance() -> FillHist1("dx", dX, 100, 0, 30);
    STDebugLogger::Instance() -> FillHist1("dy", dY, 100, 0, 30);
    STDebugLogger::Instance() -> FillHist1("dz", dZ, 100, 0, 10);
#endif

    if (dX < 0) dX *= -1;
    if (dY < 0) dY *= -1;
    if (dZ < 0) dZ *= -1;

#ifdef DEBUGLIT_BUILD
    cout << "prox position: " 
         << positionHitInTrack.X() << " & " << positionHit.X() << ", "
         << positionHitInTrack.Y() << " & " << positionHit.Y() << ", "
         << positionHitInTrack.Z() << " & " << positionHit.Z() << ", "
         << endl;
#endif

#ifdef DEBUGLIT_BUILD
    cout << "prox pos diff: " 
         << dX << " <? " << xCut << ", "
         << dY << " <? " << yCut << ", "
         << dZ << " <? " << zCut << endl;
#endif

    Double_t corrX = (xCut - dX) / xCut;
    Double_t corrY = (yCut - dY) / yCut;
    Double_t corrZ = (zCut - dZ) / zCut;

    if (corrX >= 0 && corrY >= 0 && corrZ >= 0) 
    {
      survive = kTRUE;
      matchQuality = TMath::Sqrt(corrX*corrX + corrY*corrY + corrZ*corrZ + matchQuality) 
                     / (corrX + corrY + corrZ + matchQuality);
      return kTRUE;
    }
  }

  return kTRUE;
}
