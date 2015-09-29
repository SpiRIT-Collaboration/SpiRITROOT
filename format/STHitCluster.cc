/**
 *  @brief STHit Class
 *
 *  @author Genie Jhang ( geniejhang@majimak.com )
 *  @author JungWoo Lee
 */

#include "STHitCluster.hh"

#include "TMath.h"

#include <iostream>

ClassImp(STHitCluster);

STHitCluster::STHitCluster()
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
}

STHitCluster::STHitCluster(STHitCluster *cluster)
{
  fClusterID = cluster -> GetClusterID();

  fX = cluster -> GetX();
  fY = cluster -> GetZ();
  fZ = cluster -> GetZ();
  fDx = cluster -> GetDx();
  fDy = cluster -> GetDy();
  fDz = cluster -> GetDz();

  fCovMatrix.ResizeTo(3, 3);
  fCovMatrix = cluster -> GetCovMatrix();

  fHitIDArray = *(cluster -> GetHitIDs());

  fCharge = cluster -> GetCharge();
}

STHitCluster::~STHitCluster()
{
}

void STHitCluster::SetCovMatrix(TMatrixD matrix) { fCovMatrix = matrix; } 

     TMatrixD  STHitCluster::GetCovMatrix() const  { return fCovMatrix; }
        Int_t  STHitCluster::GetNumHits()          { return fHitIDArray.size(); }
vector<Int_t> *STHitCluster::GetHitIDs()           { return &fHitIDArray; }

void 
STHitCluster::AddHit(STHit *hit)
{
  TVector3 hitPos = hit -> GetPosition();
  Double_t charge = hit -> GetCharge();

  CalculatePosition(hitPos, charge);

  if (GetNumHits() > 0)
    CalculateCovMatrix(hitPos, charge);
  else {
    fCovMatrix(0, 0) = hit -> GetDx();
    fCovMatrix(1, 1) = hit -> GetDy();
    fCovMatrix(2, 2) = hit -> GetDz();
  }

  fCharge += charge;

  fHitIDArray.push_back(hit -> GetHitID());
  hit -> SetClusterID(fClusterID);
}

void 
STHitCluster::CalculatePosition(TVector3 hitPos, Double_t charge)
{
  TVector3 position(fX, fY, fZ);

  for (Int_t iPos = 0; iPos < 3; iPos++)
    position[iPos] += charge*(hitPos[iPos] - position[iPos])/(fCharge + charge);

  fX = position.X();
  fY = position.Y();
  fZ = position.Z();
}

void 
STHitCluster::CalculateCovMatrix(TVector3 hitPos, Double_t charge)
{
  TVector3 position(fX, fY, fZ);

  for (Int_t iFirst = 0; iFirst < 3; iFirst++) {
    for (Int_t iSecond = 0; iSecond < iFirst + 1; iSecond++) {
      fCovMatrix(iFirst, iSecond) = fCharge*fCovMatrix(iFirst, iSecond)/(fCharge + charge);
      fCovMatrix(iFirst, iSecond) += charge*(hitPos[iFirst] 
                                     - position[iFirst])*(hitPos[iSecond] 
                                     - position[iSecond])/fCharge;
      fCovMatrix(iSecond, iFirst) = fCovMatrix(iFirst, iSecond);
    }
  }

  fDx = fCovMatrix(0, 0);
  fDy = fCovMatrix(1, 1);
  fDz = fCovMatrix(2, 2);
}

/*
 *
/// Without using charge information, e.g. normal mean, rms, and covariance
void STHitCluster::CalculatePosition(TVector3 hitPos)
{
  for (Int_t iPos = 0; iPos < 3; iPos++)
    fPosition[iPos] += (hitPos[iPos] - fPosition[iPos])/(Double_t)(GetNumHits() + 1);
}

void STHitCluster::CalculateCovMatrix(TVector3 hitPos)
{
  for (Int_t iFirst = 0; iFirst < 3; iFirst++) {
    for (Int_t iSecond = 0; iSecond < iFirst + 1; iSecond++) {
      fCovMatrix(iFirst, iSecond) = GetNumHits()*fCovMatrix(iFirst, iSecond)/(Double_t)(GetNumHits() + 1);
      fCovMatrix(iFirst, iSecond) += (hitPos[iFirst] - fPosition[iFirst])
                                     * (hitPos[iSecond] - fPosition[iSecond])
                                     / (Double_t)(GetNumHits());
      fCovMatrix(iSecond, iFirst) = fCovMatrix(iFirst, iSecond);
    }

    fPosSigma[iFirst] = TMath::Sqrt(fCovMatrix(iFirst, iFirst));
  }
}
*/
