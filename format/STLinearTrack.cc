/**
 * @brief SpiRITROOT linear track container
 *
 * @author JungWoo Lee
 */

#include "STLinearTrack.hh"

ClassImp(STLinearTrack)

STLinearTrack::STLinearTrack()
{
  Clear();
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

  fExpectationX = track -> GetExpectationX();
  fExpectationY = track -> GetExpectationY();
  fExpectationZ = track -> GetExpectationZ();

  fExpectationXX = track -> GetExpectationXX();
  fExpectationYY = track -> GetExpectationYY();
  fExpectationZZ = track -> GetExpectationZZ();

  fExpectationXY = track -> GetExpectationXY();
  fExpectationYZ = track -> GetExpectationYZ();
  fExpectationZX = track -> GetExpectationZX();
}

STLinearTrack::STLinearTrack(Int_t trackID, STHit* hit)
{
  Clear();
  fTrackID = trackID;
  AddHit(hit);
}

void STLinearTrack::Clear(Option_t *option)
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

  fExpectationX = 0;
  fExpectationY = 0;
  fExpectationZ = 0;

  fExpectationXX = 0;
  fExpectationYY = 0;
  fExpectationZZ = 0;

  fExpectationXY = 0;
  fExpectationYZ = 0;
  fExpectationZX = 0;

  fHitIDArray.clear();

  if (TString(option) == "C")
    DeleteHits();
  else
    fHitPtrArray.clear();
}

void STLinearTrack::DeleteHits()
{
  UInt_t size1 = fHitPtrArray.size();
  for (UInt_t iHit = 0; iHit < size1; iHit++)
    delete fHitPtrArray[iHit];

  fHitPtrArray.clear();
}

void STLinearTrack::AddHit(STHit *hit)
{
  fIsFitted = kFALSE;
  fIsSorted = kFALSE;

  TVector3 position = hit -> GetPosition();
  Double_t charge   = hit -> GetCharge();

  fExpectationX = (fChargeSum * fExpectationX + charge * position.X()) / (fChargeSum + charge);
  fExpectationY = (fChargeSum * fExpectationY + charge * position.Y()) / (fChargeSum + charge);
  fExpectationZ = (fChargeSum * fExpectationZ + charge * position.Z()) / (fChargeSum + charge);

  fExpectationXX = (fChargeSum * fExpectationXX + charge * position.X() * position.X()) / (fChargeSum + charge);
  fExpectationYY = (fChargeSum * fExpectationYY + charge * position.Y() * position.Y()) / (fChargeSum + charge);
  fExpectationZZ = (fChargeSum * fExpectationZZ + charge * position.Z() * position.Z()) / (fChargeSum + charge);

  fExpectationXY = (fChargeSum * fExpectationXY + charge * position.X() * position.Y()) / (fChargeSum + charge);
  fExpectationYZ = (fChargeSum * fExpectationYZ + charge * position.Y() * position.Z()) / (fChargeSum + charge);
  fExpectationZX = (fChargeSum * fExpectationZX + charge * position.Z() * position.X()) / (fChargeSum + charge);

  fChargeSum += charge;

  fHitPtrArray.push_back(hit);
  fHitIDArray.push_back(hit -> GetHitID());

  fNumHits++;
}

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
  fExpectationX = pos.X();
  fExpectationY = pos.Y();
  fExpectationZ = pos.Z();
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

   Int_t STLinearTrack::GetTrackID()    const { return fTrackID; }
  Bool_t STLinearTrack::IsPrimary()     const { return fIsPrimary; }
  Bool_t STLinearTrack::IsFitted()      const { return fIsFitted; }
  Bool_t STLinearTrack::IsSorted()      const { return fIsSorted; }

TVector3 STLinearTrack::GetVertex(Int_t ith)     const { return TVector3(fXVertex[ith], fYVertex[ith], fZVertex[ith]); }
TVector3 STLinearTrack::GetDirection()  const { return TVector3(fXDirection, fYDirection, fZDirection); }
TVector3 STLinearTrack::GetNormal()     const { return TVector3(fXNormal, fYNormal, fZNormal); }
TVector3 STLinearTrack::GetCentroid()   const { return TVector3(fExpectationX, fExpectationY, fExpectationZ); }

Double_t STLinearTrack::GetXVertex(Int_t ith)    const { return fXVertex[ith]; }
Double_t STLinearTrack::GetYVertex(Int_t ith)    const { return fYVertex[ith]; }
Double_t STLinearTrack::GetZVertex(Int_t ith)    const { return fZVertex[ith]; }

Double_t STLinearTrack::GetXDirection() const { return fXDirection; }
Double_t STLinearTrack::GetYDirection() const { return fYDirection; }
Double_t STLinearTrack::GetZDirection() const { return fZDirection; }

Double_t STLinearTrack::GetXNormal()    const { return fXNormal; }
Double_t STLinearTrack::GetYNormal()    const { return fYNormal; }
Double_t STLinearTrack::GetZNormal()    const { return fZNormal; }

Double_t STLinearTrack::GetXCentroid()  const { return fExpectationX; }
Double_t STLinearTrack::GetYCentroid()  const { return fExpectationY; }
Double_t STLinearTrack::GetZCentroid()  const { return fExpectationZ; }

Double_t STLinearTrack::GetChargeSum()  const { return fChargeSum; }
   Int_t STLinearTrack::GetNumHits()          { return fNumHits; }
   Int_t STLinearTrack::GetNumHitsRemoved()   { return 0; }

std::vector<Int_t>  *STLinearTrack::GetHitIDArray()      { return &fHitIDArray; }
std::vector<STHit*> *STLinearTrack::GetHitPointerArray() { return &fHitPtrArray; }

Int_t  STLinearTrack::GetHitID(Int_t i) { return fHitIDArray.at(i); }
STHit* STLinearTrack::GetHit(Int_t i)   { return fHitPtrArray.at(i); }

Double_t STLinearTrack::GetRMSLine()    const { return fRMSLine; }
Double_t STLinearTrack::GetRMSPlane()   const { return fRMSPlane; }

Double_t STLinearTrack::GetSumDistCXX() const { return fChargeSum * (fExpectationXX - fExpectationX * fExpectationX); }
Double_t STLinearTrack::GetSumDistCYY() const { return fChargeSum * (fExpectationYY - fExpectationY * fExpectationY); }
Double_t STLinearTrack::GetSumDistCZZ() const { return fChargeSum * (fExpectationZZ - fExpectationZ * fExpectationZ); }

Double_t STLinearTrack::GetSumDistCXY() const { return fChargeSum * (fExpectationXY - fExpectationX * fExpectationY); }
Double_t STLinearTrack::GetSumDistCYZ() const { return fChargeSum * (fExpectationYZ - fExpectationY * fExpectationZ); }
Double_t STLinearTrack::GetSumDistCZX() const { return fChargeSum * (fExpectationZX - fExpectationZ * fExpectationX); }

Double_t STLinearTrack::GetExpectationX()  const { return fExpectationX; }
Double_t STLinearTrack::GetExpectationY()  const { return fExpectationY; }
Double_t STLinearTrack::GetExpectationZ()  const { return fExpectationZ; }

Double_t STLinearTrack::GetExpectationXX() const { return fExpectationXX; }
Double_t STLinearTrack::GetExpectationYY() const { return fExpectationYY; }
Double_t STLinearTrack::GetExpectationZZ() const { return fExpectationZZ; }

Double_t STLinearTrack::GetExpectationXY() const { return fExpectationXY; }
Double_t STLinearTrack::GetExpectationYZ() const { return fExpectationYZ; }
Double_t STLinearTrack::GetExpectationZX() const { return fExpectationZX; }
