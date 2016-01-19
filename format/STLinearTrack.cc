/**
 * @brief SpiRITROOT linear track container
 *
 * @author JungWoo Lee
 */

#include "STLinearTrack.hh"

ClassImp(STLinearTrack)

STLinearTrack::STLinearTrack()
{
  Reset();
}

STLinearTrack::~STLinearTrack()
{
}

STLinearTrack::STLinearTrack(STLinearTrack *track)
{
  fTrackID   = track -> GetTrackID();
  fIsPrimary = track -> IsPrimary();
  fIsFitted  = track -> IsFitted();
  fIsSorted  = track -> IsSorted();

  fXVertex[0] = track -> GetXVertex(0);
  fYVertex[0] = track -> GetYVertex(0);
  fZVertex[0] = track -> GetZVertex(0);

  fXVertex[1] = track -> GetXVertex(1);
  fYVertex[1] = track -> GetYVertex(1);
  fZVertex[1] = track -> GetZVertex(1);

  fXCentroid = track -> GetXCentroid();
  fYCentroid = track -> GetYCentroid();
  fZCentroid = track -> GetZCentroid();

  fXDirection = track -> GetXDirection();
  fYDirection = track -> GetYDirection();
  fZDirection = track -> GetZDirection();

  fXNormal = track -> GetXNormal();
  fYNormal = track -> GetYNormal();
  fZNormal = track -> GetZNormal();

  fChargeSum = track -> GetChargeSum();
  fNumHits = track -> GetNumHits();

  fHitPtrArray = *(track -> GetHitPointerArray()); 
  fHitIDArray  = *(track -> GetHitIDArray()); 

  fRMSLine  = track -> GetRMSLine();
  fRMSPlane = track -> GetRMSPlane();

  /*
  fSumDistCX = track -> GetSumDistCX();
  fSumDistCY = track -> GetSumDistCY();
  fSumDistCZ = track -> GetSumDistCZ();

  fSumDistCXX = track -> GetSumDistCXX();
  fSumDistCYY = track -> GetSumDistCYY();
  fSumDistCZZ = track -> GetSumDistCZZ();

  fSumDistCXY = track -> GetSumDistCXY();
  fSumDistCYZ = track -> GetSumDistCYZ();
  fSumDistCZX = track -> GetSumDistCZX();
  */
}

STLinearTrack::STLinearTrack(Int_t trackID, STHit* hit)
{
  Reset();
  fTrackID = trackID;
  AddHit(hit);
}

void STLinearTrack::Reset()
{
  fTrackID   = -1;
  fIsPrimary = kFALSE;
  fIsFitted  = kFALSE;
  fIsSorted  = kFALSE;

  fXVertex[0] = 0;
  fYVertex[0] = 0;
  fZVertex[0] = 0;

  fXVertex[1] = 0;
  fYVertex[1] = 0;
  fZVertex[1] = 0;

  fXCentroid = 0;
  fYCentroid = 0;
  fZCentroid = 0;

  fXDirection = 0;
  fYDirection = 0;
  fZDirection = 0;

  fXNormal = 0;
  fYNormal = 0;
  fZNormal = 0;

  fChargeSum = 0;
  fNumHits = 0;

  fRMSLine = -1;
  fRMSPlane = -1;

  fSumDistCX = 0;
  fSumDistCY = 0;
  fSumDistCZ = 0;

  fSumDistCXX = 0;
  fSumDistCYY = 0;
  fSumDistCZZ = 0;

  fSumDistCXY = 0;
  fSumDistCYZ = 0;
  fSumDistCZX = 0;
}

////////////////////////////////////////////////////////////////////////////////
void STLinearTrack::AddHit(STHit *hit)
{
  fIsFitted = kFALSE;
  fIsSorted = kFALSE;

  Double_t xCentroid_pre = fXCentroid;
  Double_t yCentroid_pre = fYCentroid;
  Double_t zCentroid_pre = fZCentroid;
  Double_t chargeSum_pre = fChargeSum;

  TVector3 position = hit -> GetPosition();
  Double_t charge   = hit -> GetCharge();

  fXCentroid = (fXCentroid * fChargeSum  +  charge * position.X());
  fYCentroid = (fYCentroid * fChargeSum  +  charge * position.Y());
  fZCentroid = (fZCentroid * fChargeSum  +  charge * position.Z());

  fChargeSum += charge;

  fXCentroid *= 1./fChargeSum;
  fYCentroid *= 1./fChargeSum;
  fZCentroid *= 1./fChargeSum;

  fHitPtrArray.push_back(hit);
  fHitIDArray.push_back(hit -> GetHitID());

  fNumHits++;

  Double_t dXCentroid = fXCentroid - xCentroid_pre;
  Double_t dYCentroid = fYCentroid - yCentroid_pre;
  Double_t dZCentroid = fZCentroid - zCentroid_pre;

  Double_t dXNew = position.X() - fXCentroid;
  Double_t dYNew = position.Y() - fYCentroid;
  Double_t dZNew = position.Z() - fZCentroid;

  fSumDistCXX += charge * dXNew * dXNew + (chargeSum_pre * dXCentroid - 2 * fSumDistCX) * dXCentroid;
  fSumDistCYY += charge * dYNew * dYNew + (chargeSum_pre * dYCentroid - 2 * fSumDistCY) * dYCentroid;
  fSumDistCZZ += charge * dZNew * dZNew + (chargeSum_pre * dZCentroid - 2 * fSumDistCZ) * dZCentroid;

  fSumDistCXY += charge * dXNew * dYNew - dYCentroid * fSumDistCX - dXCentroid * fSumDistCY + chargeSum_pre * dXCentroid * dYCentroid;
  fSumDistCYZ += charge * dYNew * dZNew - dZCentroid * fSumDistCY - dYCentroid * fSumDistCZ + chargeSum_pre * dYCentroid * dZCentroid;
  fSumDistCZX += charge * dZNew * dXNew - dXCentroid * fSumDistCZ - dZCentroid * fSumDistCX + chargeSum_pre * dZCentroid * dXCentroid;

  fSumDistCX += charge * dXNew - chargeSum_pre * dXCentroid;
  fSumDistCY += charge * dYNew - chargeSum_pre * dYCentroid;
  fSumDistCZ += charge * dZNew - chargeSum_pre * dZCentroid;
}
////////////////////////////////////////////////////////////////////////////////

void STLinearTrack::SetTrackID(Int_t id)     { fTrackID   = id;  }
void STLinearTrack::SetIsPrimary(Bool_t val) { fIsPrimary = val; }
void STLinearTrack::SetIsFitted(Bool_t val)  { fIsFitted  = val; }
void STLinearTrack::SetIsSorted(Bool_t val)  { fIsSorted  = val; }

void STLinearTrack::SetVertex(Int_t ith, TVector3 pos)
{
  fXVertex[ith] = pos.X();
  fYVertex[ith] = pos.Y();
  fZVertex[ith] = pos.Z();
}
void STLinearTrack::SetDirection(TVector3 vec)
{
  fXDirection = vec.X();
  fYDirection = vec.Y();
  fZDirection = vec.Z();
}
void STLinearTrack::SetNormal(TVector3 vec)
{
  fXNormal = vec.X();
  fYNormal = vec.Y();
  fZNormal = vec.Z();
}
void STLinearTrack::SetCentroid(TVector3 pos)
{
  fXCentroid = pos.X();
  fYCentroid = pos.Y();
  fZCentroid = pos.Z();
}

void STLinearTrack::SetXVertex(Int_t ith, Double_t x) { fXVertex[ith] = x; }
void STLinearTrack::SetYVertex(Int_t ith, Double_t y) { fYVertex[ith] = y; }
void STLinearTrack::SetZVertex(Int_t ith, Double_t z) { fZVertex[ith] = z; }

void STLinearTrack::SetXDirection(Double_t x) { fXDirection = x; }
void STLinearTrack::SetYDirection(Double_t y) { fYDirection = y; }
void STLinearTrack::SetZDirection(Double_t z) { fZDirection = z; }

void STLinearTrack::SetXNormal(Double_t x) { fXNormal = x; }
void STLinearTrack::SetYNormal(Double_t y) { fYNormal = y; }
void STLinearTrack::SetZNormal(Double_t z) { fZNormal = z; }

void STLinearTrack::SetChargeSum(Double_t val) { fChargeSum = val; }
void STLinearTrack::SetNumHits(Int_t val)      { fNumHits = val; }

void STLinearTrack::SetRMSLine (Double_t rms) { fRMSLine  = rms; }
void STLinearTrack::SetRMSPlane(Double_t rms) { fRMSPlane = rms; }

void STLinearTrack::SetSumDistCX(Double_t val) { fSumDistCX = val; }
void STLinearTrack::SetSumDistCY(Double_t val) { fSumDistCY = val; }
void STLinearTrack::SetSumDistCZ(Double_t val) { fSumDistCZ = val; }

void STLinearTrack::SetSumDistCXX(Double_t val) { fSumDistCXX = val; }
void STLinearTrack::SetSumDistCYY(Double_t val) { fSumDistCYY = val; }
void STLinearTrack::SetSumDistCZZ(Double_t val) { fSumDistCZZ = val; }

void STLinearTrack::SetSumDistCXY(Double_t val) { fSumDistCXY = val; }
void STLinearTrack::SetSumDistCYZ(Double_t val) { fSumDistCYZ = val; }
void STLinearTrack::SetSumDistCZX(Double_t val) { fSumDistCZX = val; }

   Int_t STLinearTrack::GetTrackID()    const { return fTrackID; }
  Bool_t STLinearTrack::IsPrimary()     const { return fIsPrimary; }
  Bool_t STLinearTrack::IsFitted()      const { return fIsFitted; }
  Bool_t STLinearTrack::IsSorted()      const { return fIsSorted; }

TVector3 STLinearTrack::GetVertex(Int_t ith)     const { return TVector3(fXVertex[ith], fYVertex[ith], fZVertex[ith]); }
TVector3 STLinearTrack::GetDirection()  const { return TVector3(fXDirection, fYDirection, fZDirection); }
TVector3 STLinearTrack::GetNormal()     const { return TVector3(fXNormal, fYNormal, fZNormal); }
TVector3 STLinearTrack::GetCentroid()   const { return TVector3(fXCentroid, fYCentroid, fZCentroid); }

Double_t STLinearTrack::GetXVertex(Int_t ith)    const { return fXVertex[ith]; }
Double_t STLinearTrack::GetYVertex(Int_t ith)    const { return fYVertex[ith]; }
Double_t STLinearTrack::GetZVertex(Int_t ith)    const { return fZVertex[ith]; }

Double_t STLinearTrack::GetXDirection() const { return fXDirection; }
Double_t STLinearTrack::GetYDirection() const { return fYDirection; }
Double_t STLinearTrack::GetZDirection() const { return fZDirection; }

Double_t STLinearTrack::GetXNormal()    const { return fXNormal; }
Double_t STLinearTrack::GetYNormal()    const { return fYNormal; }
Double_t STLinearTrack::GetZNormal()    const { return fZNormal; }

Double_t STLinearTrack::GetXCentroid()  const { return fXCentroid; }
Double_t STLinearTrack::GetYCentroid()  const { return fYCentroid; }
Double_t STLinearTrack::GetZCentroid()  const { return fZCentroid; }

Double_t STLinearTrack::GetChargeSum()  const { return fChargeSum; }
   Int_t STLinearTrack::GetNumHits()          { return fNumHits; }

std::vector<Int_t>  *STLinearTrack::GetHitIDArray()      { return &fHitIDArray; }
std::vector<STHit*> *STLinearTrack::GetHitPointerArray() { return &fHitPtrArray; }

Int_t  STLinearTrack::GetHitID(Int_t i) { return fHitIDArray.at(i); }
STHit* STLinearTrack::GetHit(Int_t i)   { return fHitPtrArray.at(i); }

Double_t STLinearTrack::GetRMSLine()    const { return fRMSLine; }
Double_t STLinearTrack::GetRMSPlane()   const { return fRMSPlane; }

Double_t STLinearTrack::GetSumDistCX()  const { return fSumDistCX; }
Double_t STLinearTrack::GetSumDistCY()  const { return fSumDistCY; }
Double_t STLinearTrack::GetSumDistCZ()  const { return fSumDistCZ; }

Double_t STLinearTrack::GetSumDistCXX() const { return fSumDistCXX; }
Double_t STLinearTrack::GetSumDistCYY() const { return fSumDistCYY; }
Double_t STLinearTrack::GetSumDistCZZ() const { return fSumDistCZZ; }

Double_t STLinearTrack::GetSumDistCXY() const { return fSumDistCXY; }
Double_t STLinearTrack::GetSumDistCYZ() const { return fSumDistCYZ; }
Double_t STLinearTrack::GetSumDistCZX() const { return fSumDistCZX; }
