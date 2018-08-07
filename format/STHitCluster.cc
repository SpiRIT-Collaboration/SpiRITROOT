/**
 *  @brief STHit Class
 *
 *  @author Genie Jhang ( geniejhang@majimak.com )
 *  @author JungWoo Lee
 *
 *  method IsClustered() is always true.
 *  fIsClustered is stable flag for STHitCluster.
 */

#include "STHitCluster.hh"

#include "TMath.h"

#include <iostream>

ClassImp(STHitCluster);

STHitCluster::STHitCluster()
{
  Clear();
}

STHitCluster::STHitCluster(STHitCluster *cluster)
{
  fIsClustered = cluster -> IsClustered();
  fClusterID = cluster -> GetClusterID();
  fTrackID = cluster -> GetTrackID();
  fIsMissingCharge =  cluster -> IsMissingCharge();
  
  fX = cluster -> GetX();
  fY = cluster -> GetY();
  fZ = cluster -> GetZ();
  fDx = cluster -> GetDx();
  fDy = cluster -> GetDy();
  fDz = cluster -> GetDz();

  fCovMatrix.ResizeTo(3, 3);
  fCovMatrix = cluster -> GetCovMatrix();

  fHitIDArray  = *(cluster -> GetHitIDs());
  fHitPtrArray = *(cluster -> GetHitPtrs());
  fIsEmbed = cluster -> IsEmbed();
  fCharge  = cluster -> GetCharge();
  fLength  = cluster -> GetLength();

  SetPOCA(cluster -> GetPOCA());

  fIsContinuousHits = cluster -> IsContinuousHits();
}

void STHitCluster::Clear(Option_t *)
{
  STHit::Clear();

  fClusterID = -1;

  fX = 0;
  fY = 0;
  fZ = 0;
  fDx = 0;
  fDy = 0;
  fDz = 0;

  fCharge = 0;

  fCovMatrix.ResizeTo(3, 3);
  for (Int_t iElem = 0; iElem < 9; iElem++)
    fCovMatrix(iElem/3, iElem%3) = 0;

  fHitIDArray.clear();
  fHitPtrArray.clear();

  fLength = -999;

  fPOCAX = -999;
  fPOCAY = -999;
  fPOCAZ = -999;

  fS = 0;

  fIsEmbed         = false;
  fIsClustered     = kFALSE;
  fIsMissingCharge = false;

  fIsContinuousHits = kFALSE;
}

void STHitCluster::SetCovMatrix(TMatrixD matrix) { fCovMatrix = matrix; } 

Bool_t STHitCluster::IsClustered() const { return kTRUE; }
Int_t  STHitCluster::GetHitID()    const { return fClusterID; }

TMatrixD   STHitCluster::GetCovMatrix()        const  { return fCovMatrix; }
Int_t   STHitCluster::GetNumHits()             const { return fHitIDArray.size(); }
Int_t   STHitCluster::GetNumSatNeighbors()     const{ return fnumSatNeigh; }
Double_t   STHitCluster::GetFracSatNeighbors() const{ return fFractionSat; }
vector<Int_t>  *STHitCluster::GetHitIDs()           { return &fHitIDArray; }
vector<STHit*> *STHitCluster::GetHitPtrs()          { return &fHitPtrArray; }

void
STHitCluster::AddHit(STHit *hit)
{
  Double_t cov_default[3] = {4*4,1*1,6*6};

  fIsEmbed = hit -> IsEmbed(); //if hit is embeded then cluster is embeded

  auto charge = hit -> GetCharge();
  auto chargeSum = fCharge + charge;

  for (int i = 0; i < 3; ++i)
    operator[](i) = (fCharge*operator[](i) + charge*(*hit)[i]) / chargeSum;

  fS = (fCharge * fS + charge * hit -> GetS()) / chargeSum;

  if (GetNumHits() == 0) {
    for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      fCovMatrix(i,j) = 0;

    for (int i = 0; i < 3; ++i)
      fCovMatrix(i,i) = cov_default[i];
  }
  else if (GetNumHits() == 1) {
    for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      fCovMatrix(i,j) = charge*(operator[](i)-(*hit)[i])*(operator[](j)-(*hit)[j])/fCharge;
  }
  else {
    for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      fCovMatrix(i,j) = fCharge*fCovMatrix(i,j)/chargeSum + charge*(operator[](i)-(*hit)[i])*(operator[](j)-(*hit)[j])/fCharge;
  }

       if (fLayer != -1) fCovMatrix(2,2) = cov_default[2];
  else if (fRow   != -1) fCovMatrix(0,0) = cov_default[0];

  fCharge = chargeSum;

  fDx = sqrt(fCovMatrix(0, 0));
  fDy = sqrt(fCovMatrix(1, 1));
  fDz = sqrt(fCovMatrix(2, 2));

  fHitIDArray.push_back(hit -> GetHitID());
  fHitPtrArray.push_back(hit);
  hit -> SetClusterID(fClusterID);
}

void STHitCluster::ApplyModifiedHitInfo()
{
  fX = 0;
  fY = 0;
  fZ = 0;
  fDx = 0;
  fDy = 0;
  fDz = 0;
  fCharge = 0;
  fS = 0;

  fCovMatrix.ResizeTo(3, 3);
  for (Int_t iElem = 0; iElem < 9; iElem++)
    fCovMatrix(iElem/3, iElem%3) = 0;

  Double_t cov_default[3] = {4*4,1*1,6*6};

  int hit_idx = 0;
  for (auto hit : fHitPtrArray)
  {
    auto charge = hit -> GetCharge();
    auto chargeSum = fCharge + charge;

    for (int i = 0; i < 3; ++i)
      operator[](i) = (fCharge*operator[](i) + charge*(*hit)[i]) / chargeSum;

    fS = (fCharge * fS + charge * hit -> GetS()) / chargeSum;

    if (hit_idx == 0) {
      for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
          fCovMatrix(i,j) = 0;

      for (int i = 0; i < 3; ++i)
        fCovMatrix(i,i) = cov_default[i];
    }
    else if (hit_idx == 1) {
      for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
          fCovMatrix(i,j) = charge*(operator[](i)-(*hit)[i])*(operator[](j)-(*hit)[j])/fCharge;
    }
    else {
      for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
          fCovMatrix(i,j) = fCharge*fCovMatrix(i,j)/chargeSum + charge*(operator[](i)-(*hit)[i])*(operator[](j)-(*hit)[j])/fCharge;
    }

         if (fLayer != -1) fCovMatrix(2,2) = cov_default[2];
    else if (fRow   != -1) fCovMatrix(0,0) = cov_default[0];

    fCharge = chargeSum;

    fDx = sqrt(fCovMatrix(0, 0));
    fDy = sqrt(fCovMatrix(1, 1));
    fDz = sqrt(fCovMatrix(2, 2));
    hit_idx++;
  }
}

void STHitCluster::SetDFromCovForGenfit(Double_t maxx, Double_t maxy, Double_t maxz, bool setMin)
{
//  if (setMin) {
    SetDx(0.04);
    SetDy(0.01);
    SetDz(0.04);
//    return;
//  }

//  SetDx(0.16*(abs(fCovMatrix(0,0)/fCharge/maxx)+1));
//  SetDy(0.09*(abs(fCovMatrix(1,1)/fCharge/maxy)+1));
//  SetDz(0.36*(abs(fCovMatrix(2,2)/fCharge/maxz)+1));
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

Double_t STHitCluster::GetLength() { return fLength; }
Bool_t STHitCluster::IsStable() { return fIsClustered; }
Bool_t STHitCluster::IsEmbed() const { return fIsEmbed; }
Bool_t STHitCluster::IsMissingCharge() const { return fIsMissingCharge; }

void STHitCluster::SetIsEmbed(Bool_t val) { fIsEmbed = val; }
void STHitCluster::SetIsMissingCharge(Bool_t val) { fIsMissingCharge = val; } //missing charge from dead pad by saturation
void STHitCluster::SetIsStable(Bool_t isStable)   { fIsClustered = isStable; }
void STHitCluster::SetNumSatNeighbors(Int_t num)     { fnumSatNeigh = num; }
void STHitCluster::SetFractSatNeighbors(Double_t num) { fFractionSat = num; }
void STHitCluster::SetLength(Double_t length) { fLength = length; }
void STHitCluster::SetPOCA(TVector3 p)
{
  fPOCAX = p.X();
  fPOCAY = p.Y();
  fPOCAZ = p.Z();
}
TVector3 STHitCluster::GetPOCA() { return TVector3(fPOCAX, fPOCAY, fPOCAZ); }

void STHitCluster::ApplyCovLowLimit()
{
}
