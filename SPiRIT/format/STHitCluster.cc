// =================================================
//  STHitCluster Class                          
//                                                  
//  Description:                                    
//    Container for a cluster of hits
//                                                  
//  Genie Jhang ( geniejhang@majimak.com )          
//  2014. 07. 15                                    
// =================================================

#include "STHitCluster.hh"

#include "TMath.h"

ClassImp(STHitCluster)

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

void STHitCluster::SetClusterID(Int_t clusterID) { fClusterID = clusterID; }
Int_t STHitCluster::GetClusterID()               { return fClusterID; }

TVector3 STHitCluster::GetPosition()             { return fPosition; }
TVector3 STHitCluster::GetPosSigma()             { return fPosSigma; }
TMatrixD STHitCluster::GetCovMatrix()            { return fCovMatrix; }
Double_t STHitCluster::GetCharge()               { return fCharge; }

Int_t STHitCluster::GetNumHits()                 { return fHitIDArray.size(); }
vector<Int_t> *STHitCluster::GetHitIDs()         { return &fHitIDArray; }

void STHitCluster::AddHit(STHit *hit)
{
  CalculatePosition(hit -> GetPosition(), hit -> GetCharge());

  if (GetNumHits() > 0)
    CalculateCovMatrix(hit -> GetPosition(), hit -> GetCharge());

  fCharge += hit -> GetCharge();

  fHitIDArray.push_back(hit -> GetHitID());
  hit -> SetClusterID(fClusterID);
}

void STHitCluster::CalculatePosition(TVector3 hitPos, Double_t charge)
{
  for (Int_t iPos = 0; iPos < 3; iPos++)
    fPosition[iPos] += charge*(hitPos[iPos] - fPosition[iPos])/(fCharge + charge);
}

void STHitCluster::CalculateCovMatrix(TVector3 hitPos, Double_t charge)
{
  for (Int_t iFirst = 0; iFirst < 3; iFirst++) {
    for (Int_t iSecond = 0; iSecond < iFirst + 1; iSecond++) {
      fCovMatrix(iFirst, iSecond) = fCharge*fCovMatrix(iFirst, iSecond)/(fCharge + charge);
      fCovMatrix(iFirst, iSecond) += charge*(hitPos[iFirst] - fPosition[iFirst])*(hitPos[iSecond] - fPosition[iSecond])/fCharge;
      fCovMatrix(iSecond, iFirst) = fCovMatrix(iFirst, iSecond);
    }

    fPosSigma[iFirst] = TMath::Sqrt(fCovMatrix(iFirst, iFirst));
  }
}

/*
// Without using charge information, e.g. normal mean, rms, and covariance
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
      fCovMatrix(iFirst, iSecond) += (hitPos[iFirst] - fPosition[iFirst])*(hitPos[iSecond] - fPosition[iSecond])/(Double_t)(GetNumHits());
      fCovMatrix(iSecond, iFirst) = fCovMatrix(iFirst, iSecond);
    }

    fPosSigma[iFirst] = TMath::Sqrt(fCovMatrix(iFirst, iFirst));
  }
}
*/
