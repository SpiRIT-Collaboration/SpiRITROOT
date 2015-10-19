#include "STCorrLinearTHProx.hh"

STCorrLinearTHProx::STCorrLinearTHProx(
  Int_t nHitsCompare,
  Double_t xProxCut,
  Double_t yProxCut,
  Double_t zProxCut)
{
  fNumHitsCompare = nHitsCompare;
  fXProxCut = xProxCut;
  fYProxCut = yProxCut;
  fZProxCut = zProxCut;
}

Bool_t 
STCorrLinearTHProx::Correlate(STLinearTrack *track, 
                              STHit *hit, 
                              Bool_t &survive, 
                              Double_t &matchQuality)
{
  survive = kFALSE;
  Double_t scaling = 2 - matchQuality;

  Double_t xCut = scaling * fXProxCut;
  Double_t yCut = scaling * fYProxCut;
  Double_t zCut = scaling * fZProxCut;

  Int_t numHitsInTrack = track -> GetNumHits();
  Int_t nHitsCompare = numHitsInTrack;
  if (nHitsCompare > fNumHitsCompare) 
    nHitsCompare = fNumHitsCompare;

  TVector3 positionHit = hit -> GetPosition();

  for (Int_t iHit = 0; iHit < nHitsCompare; iHit++)
  {
    STHit *hitT = track -> GetHit(numHitsInTrack - 1 - iHit);
    TVector3 positionHitInTrack = hitT -> GetPosition();

    Double_t dX = positionHitInTrack.X() - positionHit.X();
    Double_t dY = positionHitInTrack.Y() - positionHit.Y();
    Double_t dZ = positionHitInTrack.Z() - positionHit.Z();

    if (dX < 0) dX *= -1;
    if (dY < 0) dY *= -1;
    if (dZ < 0) dZ *= -1;

    Double_t corrX = (xCut - dX) / xCut;
    Double_t corrY = (yCut - dX) / yCut;
    Double_t corrZ = (zCut - dX) / zCut;

    if (corrX <= 0 && corrY <= 0 && corrZ <= 0) 
    {
      survive = kTRUE;
      matchQuality = TMath::Sqrt(corrX*corrX + corrY*corrY + corrZ*corrZ);
      return kTRUE;
    }
  }

  return kTRUE;
}
