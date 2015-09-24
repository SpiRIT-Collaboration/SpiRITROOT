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

  fPosition = TVector3(0, 0, -1000);
  fPosSigma = TVector3(0., 0., 0.);

  fCovMatrix.ResizeTo(3, 3);
  for (Int_t iElem = 0; iElem < 9; iElem++)
    fCovMatrix(iElem/3, iElem%3) = 0;

  fCharge = 0.;
}

STHitCluster::STHitCluster(STHitCluster *cluster)
{
  fClusterID = cluster -> GetClusterID();

  fPosition = cluster -> GetPosition();
  fPosSigma = cluster -> GetPosSigma();

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
    TVector3 posSigma = hit -> GetPosSigma();
    fCovMatrix(0, 0) = posSigma.X();
    fCovMatrix(1, 1) = posSigma.Y();
    fCovMatrix(2, 2) = posSigma.Z();
  }

  fCharge += charge;

  fHitIDArray.push_back(hit -> GetHitID());
  hit -> SetClusterID(fClusterID);
}

void 
STHitCluster::CalculatePosition(TVector3 hitPos, Double_t charge)
{
  for (Int_t iPos = 0; iPos < 3; iPos++)
    fPosition[iPos] += charge*(hitPos[iPos] - fPosition[iPos])/(fCharge + charge);
}

void 
STHitCluster::CalculateCovMatrix(TVector3 hitPos, Double_t charge)
{
  for (Int_t iFirst = 0; iFirst < 3; iFirst++) {
    for (Int_t iSecond = 0; iSecond < iFirst + 1; iSecond++) {
      fCovMatrix(iFirst, iSecond) = fCharge*fCovMatrix(iFirst, iSecond)/(fCharge + charge);
      fCovMatrix(iFirst, iSecond) += charge*(hitPos[iFirst] 
                                     - fPosition[iFirst])*(hitPos[iSecond] 
                                     - fPosition[iSecond])/fCharge;
      fCovMatrix(iSecond, iFirst) = fCovMatrix(iFirst, iSecond);
    }
    fPosSigma[iFirst] = TMath::Sqrt(fCovMatrix(iFirst, iFirst));
  }
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
