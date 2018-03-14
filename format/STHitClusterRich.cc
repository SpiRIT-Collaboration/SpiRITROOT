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

  fXMax = -9999;
  fYMax = -9999;
  fZMax = -9999;

  fXMin = 9999;
  fYMin = 9999;
  fZMin = 9999;
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
  STHitCluster::AddHit(hit);

  if (GetNumHits() > 0)
  {
    if (hit->GetX() > fXMax) fXMax = hit->GetX();
    if (hit->GetX() < fXMin) fXMin = hit->GetX();

    if (hit->GetY() > fYMax) fYMax = hit->GetY();
    if (hit->GetY() < fYMin) fYMin = hit->GetY();

    if (hit->GetZ() > fZMax) fZMax = hit->GetZ();
    if (hit->GetZ() < fZMin) fZMin = hit->GetZ();
  }
  else
  {
    fXMax = hit->GetX();
    fXMin = hit->GetX();

    fYMax = hit->GetY();
    fYMin = hit->GetY();

    fZMax = hit->GetZ();
    fZMin = hit->GetZ();
  }
}
