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

#include <iostream>

ClassImp(STHitCluster);

STHitCluster::STHitCluster()
{
  fClusterID = -1;

  fPosition = TVector3(0, 0, -1000);
  fPosSigma = TVector3(0., 0., 0.);

  fPhi = 0;

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

  fPhi = cluster -> Phi();

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

void STHitCluster::SetPosition(TVector3 vector)  { fPosition = vector; }
void STHitCluster::SetPosSigma(TVector3 vector)  { fPosSigma = vector; } 
void STHitCluster::SetCovMatrix(TMatrixD matrix) { fCovMatrix = matrix; } 
void STHitCluster::SetCharge(Double_t charge)    { fCharge = charge; } 

Double_t STHitCluster::Phi()                     { return fPhi; } 

TVector3 STHitCluster::GetPosition() const       { return fPosition; }
TVector3 STHitCluster::GetPosSigma() const       { return fPosSigma; }
TMatrixD STHitCluster::GetCovMatrix() const      { return fCovMatrix; }
Double_t STHitCluster::GetCharge()               { return fCharge; }

Int_t STHitCluster::GetNumHits()                 { return fHitIDArray.size(); }
vector<Int_t> *STHitCluster::GetHitIDs()         { return &fHitIDArray; }

void STHitCluster::AddHit(STHit *hit)
{
  TVector3 hitPos = hit -> GetPosition();
  Double_t charge = hit -> GetCharge();

  if (GetNumHits() > 0)
    CalculatePhi(hitPos, charge);

  CalculatePosition(hitPos, charge);

  if (GetNumHits() > 0)
    CalculateCovMatrix(hitPos, charge);

  fCharge += charge;

  fHitIDArray.push_back(hit -> GetHitID());
  hit -> SetClusterID(fClusterID);
}

void STHitCluster::CalculatePosition(TVector3 hitPos, Double_t charge)
{
  /**
    * Calculate weighted mean for cluster position. (Weight = charge) <br>
    * Weighted mean: \f$ \mu_{n+1} = \mu_n + \displaystyle\frac{a_{n+1} - \mu_n}{W_n+w_{n+1}},\quad(n\geq0). \f$
   **/

  for (Int_t iPos = 0; iPos < 3; iPos++)
    fPosition[iPos] += charge*(hitPos[iPos] - fPosition[iPos])/(fCharge + charge);
}

void STHitCluster::CalculatePhi(TVector3 hitPos, Double_t charge)
{
  if(TMath::Abs(hitPos.Z() - fPosition.Z()) > 10) 
    return;

  if(TMath::Abs(hitPos.X() - fPosition.X()) < 1) 
    return;

  Double_t phi = TMath::ATan2(hitPos.Y() - fPosition.Y(),hitPos.X() - fPosition.X());

  if (phi > TMath::Pi()/2.)
    phi -= TMath::Pi();

  if (phi < -TMath::Pi()/2.)
    phi += TMath::Pi();

  if (phi > TMath::Pi()/4. || phi < -TMath::Pi()/4.)
    return;

  fPhi += charge*(phi - fPhi)/(fCharge + charge);
}

void STHitCluster::CalculateCovMatrix(TVector3 hitPos, Double_t charge)
{
  /**
    * Calculate weighted covariance matrix for each variable. (Weight = charge) <br>
    * Cluster position uncertainty is also calculated here by taking the square root of diagonal components. <br>
    * Weighted covariance matrix: \f$\sigma(a,b)_{n+1}=\displaystyle\frac{W_n}{W_n+w_{n+1}}\sigma(a,b)_{n}+\displaystyle\frac{(\mu_{n+1}-a_{n+1})(\nu_{n+1}-b_{n+1})}{W_n},\quad(n\geq1).\f$
   **/

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
