#include "STHitClusterRich.hh"

ClassImp(STHitClusterRich)

STHitClusterRich::STHitClusterRich()
{
  fClusterID = -1;

  fX = 0;
  fY = 0;
  fZ = -1000;
  fDx = 0;
  fDy = 0;
  fDz = 0;

  fCovMatrix.ResizeTo(3, 3);
  for (Int_t iElem = 0; iElem < 9; iElem++)
    fCovMatrix(iElem/3, iElem%3) = 0;

  fCharge = 0.;

  fXMax = 0;
  fYMax = 0;
  fZMax = 0;

  fXMin = 0;
  fYMin = 0;
  fZMin = 0;
}

STHitClusterRich::STHitClusterRich(STHitClusterRich *cluster)
{
  fClusterID = cluster -> GetClusterID();

  fX = cluster -> GetX();
  fY = cluster -> GetY();
  fZ = cluster -> GetZ();
  fDx = cluster -> GetDx();
  fDy = cluster -> GetDy();
  fDz = cluster -> GetDz();

  fCovMatrix.ResizeTo(3, 3);
  fCovMatrix = cluster -> GetCovMatrix();

  fHitIDArray = *(cluster -> GetHitIDs());

  fCharge = cluster -> GetCharge();

  fXMax = cluster -> GetXMax();
  fYMax = cluster -> GetYMax();
  fZMax = cluster -> GetZMax();

  fXMin = cluster -> GetXMin();
  fYMin = cluster -> GetYMin();
  fZMin = cluster -> GetZMin();
}


Double_t STHitClusterRich::GetXMax() { return fXMax; }
Double_t STHitClusterRich::GetYMax() { return fYMax; }
Double_t STHitClusterRich::GetZMax() { return fZMax; }

Double_t STHitClusterRich::GetXMin() { return fXMin; }
Double_t STHitClusterRich::GetYMin() { return fYMin; }
Double_t STHitClusterRich::GetZMin() { return fZMin; }


void STHitClusterRich::AddHit(STHit *hit)
{
  TVector3 hitPos = hit -> GetPosition();
  Double_t charge = hit -> GetCharge();

  CalculatePosition(hitPos, charge);

  if (GetNumHits() > 0)
  {
    CalculateCovMatrix(hitPos, charge);

    if (hitPos.X() > fXMax) fXMax = hitPos.X();
    if (hitPos.X() < fXMin) fXMin = hitPos.X();

    if (hitPos.Y() > fYMax) fYMax = hitPos.Y();
    if (hitPos.Y() < fYMin) fYMin = hitPos.Y();

    if (hitPos.Z() > fZMax) fZMax = hitPos.Z();
    if (hitPos.Z() < fZMin) fZMin = hitPos.Z();
  }
  else
  {
    fXMax = hitPos.X();
    fXMin = hitPos.X();

    fYMax = hitPos.Y();
    fYMin = hitPos.Y();

    fZMax = hitPos.Z();
    fZMin = hitPos.Z();
  }

  fCharge += charge;

  fHitIDArray.push_back(hit -> GetHitID());
  hit -> SetClusterID(fClusterID);

}
