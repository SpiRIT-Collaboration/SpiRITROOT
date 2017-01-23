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
  fZ = 0;
  fDx = 0;
  fDy = 0;
  fDz = 0;

  fCovMatrix.ResizeTo(3, 3);
  for (Int_t iElem = 0; iElem < 9; iElem++)
    fCovMatrix(iElem/3, iElem%3) = 0;

  fCharge = 0.;

  fIsClustered = kFALSE;
}

STHitCluster::STHitCluster(STHitCluster *cluster)
{
  fIsClustered = cluster -> IsClustered();
  fClusterID = cluster -> GetClusterID();
  fTrackID = cluster -> GetTrackID();

  fX = cluster -> GetX();
  fY = cluster -> GetY();
  fZ = cluster -> GetZ();
  fDx = cluster -> GetDx();
  fDy = cluster -> GetDy();
  fDz = cluster -> GetDz();

  fCovMatrix.ResizeTo(3, 3);
  fCovMatrix = cluster -> GetCovMatrix();

  fHitIDArray = *(cluster -> GetHitIDs());
  fHitPtrArray = *(cluster -> GetHitPtrs());

  fCharge = cluster -> GetCharge();
}

void STHitCluster::SetCovMatrix(TMatrixD matrix) { fCovMatrix = matrix; } 

Bool_t STHitCluster::IsClustered() const { return kTRUE; }
Int_t  STHitCluster::GetHitID()    const { return fClusterID; }

     TMatrixD   STHitCluster::GetCovMatrix() const  { return fCovMatrix; }
        Int_t   STHitCluster::GetNumHits()          { return fHitIDArray.size(); }
vector<Int_t>  *STHitCluster::GetHitIDs()           { return &fHitIDArray; }
vector<STHit*> *STHitCluster::GetHitPtrs()          { return &fHitPtrArray; }

void
STHitCluster::AddHit(STHit *hit)
{
  TVector3 hitPos = hit -> GetPosition();
  Double_t charge = hit -> GetCharge();

  Int_t numHits = GetNumHits();
  if (numHits == 0) {
    fX = hitPos.X();
    fY = hitPos.Y();
    fZ = hitPos.Z();
    fCovMatrix(0, 0) = hit -> GetDx()*hit -> GetDx();
    fCovMatrix(1, 1) = hit -> GetDy()*hit -> GetDy();
    fCovMatrix(2, 2) = hit -> GetDz()*hit -> GetDz();
  }
  else {
    CalculatePosition(hitPos, charge);
    if (numHits == 1) {
      fCovMatrix(0, 0) = 0;
      fCovMatrix(1, 1) = 0;
      fCovMatrix(2, 2) = 0;
      CalculateCovMatrix(hitPos, charge);
    }
    else
      CalculateCovMatrix(hitPos, charge);

    if (fCovMatrix(0, 0) < .64)
      fCovMatrix(0, 0) = .64;
    if (fCovMatrix(2, 2) < 1.44)
      fCovMatrix(2, 2) = 1.44;
  }

  fDx = sqrt(fCovMatrix(0, 0));
  fDy = sqrt(fCovMatrix(1, 1));
  fDz = sqrt(fCovMatrix(2, 2));

  fCharge += charge;

  fHitIDArray.push_back(hit -> GetHitID());
  fHitPtrArray.push_back(hit);
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
      fCovMatrix(iFirst, iSecond) = fCharge * fCovMatrix(iFirst, iSecond) / (fCharge + charge);
      fCovMatrix(iFirst, iSecond) += charge * (hitPos[iFirst] - position[iFirst]) * (hitPos[iSecond] - position[iSecond])/fCharge;
      fCovMatrix(iSecond, iFirst) = fCovMatrix(iFirst, iSecond);
    }
  }
}

void STHitCluster::SetClusterID(Int_t clusterID)
{
  STHit::SetClusterID(clusterID);
  for (auto hit : fHitPtrArray)
    hit -> SetClusterID(clusterID);
}

void STHitCluster::SetLength(Double_t length) { fLength = length; }
Double_t STHitCluster::GetLength() { return fLength; }

void STHitCluster::SetIsStable(Bool_t isStable) { fIsClustered = isStable; }
Bool_t STHitCluster::IsStable() { return fIsClustered; }

void STHitCluster::SetPOCA(TVector3 p)
{
  fPOCAX = p.X();
  fPOCAY = p.Y();
  fPOCAZ = p.Z();
}
TVector3 STHitCluster::GetPOCA() { return TVector3(fPOCAX, fPOCAY, fPOCAZ); }
