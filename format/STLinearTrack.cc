/**
 * @brief SpiRITROOT linear track container
 *
 * @author JungWoo Lee
 */

#include "STLinearTrack.hh"

ClassImp(STLinearTrack)

STLinearTrack::STLinearTrack()
{
  fTrackID   = -1;
  fIsPrimary = kFALSE;
  fIsFitted  = kFALSE;

  fXVertex = 0;
  fYVertex = 0;
  fZVertex = 0;

  fXCentroid = 0;
  fYCentroid = 0;
  fZCentroid = 0;

  fXNormal = 0;
  fYNormal = 0;
  fZNormal = 0;

  fChargeSum = 0;

  fHitPointerArray = new std::vector<STHit*>;
  fHitIDArray = new std::vector<Int_t>;
}

STLinearTrack::~STLinearTrack()
{
}

STLinearTrack::STLinearTrack(STLinearTrack *track)
{
  fTrackID   = track -> GetTrackID();
  fIsPrimary = track -> IsPrimary();
  fIsFitted  = track -> IsFitted();

  fXVertex = track -> GetXVertex();
  fYVertex = track -> GetYVertex();
  fZVertex = track -> GetZVertex();

  fXCentroid = track -> GetXCentroid();
  fYCentroid = track -> GetYCentroid();
  fZCentroid = track -> GetZCentroid();

  fXNormal = track -> GetXNormal();
  fYNormal = track -> GetYNormal();
  fZNormal = track -> GetZNormal();

  fChargeSum = track -> GetChargeSum();

  fHitPointerArray = track -> GetHitPointerArray(); 
  fHitIDArray = track -> GetHitIDArray(); 
}

void STLinearTrack::SetTrackID(Int_t id)     { fTrackID   = id;  }
void STLinearTrack::SetIsPrimary(Bool_t val) { fIsPrimary = val; }
void STLinearTrack::SetIsFitted(Bool_t val)  { fIsFitted  = val; }

void STLinearTrack::SetVertex(TVector3 pos)
{
  fXVertex = pos.X();
  fYVertex = pos.Y();
  fZVertex = pos.Z();
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

void STLinearTrack::SetXVertex(Double_t x) { fXVertex = x; }
void STLinearTrack::SetYVertex(Double_t y) { fYVertex = y; }
void STLinearTrack::SetZVertex(Double_t z) { fZVertex = z; }

void STLinearTrack::SetXNormal(Double_t x) { fXNormal = x; }
void STLinearTrack::SetYNormal(Double_t y) { fYNormal = y; }
void STLinearTrack::SetZNormal(Double_t z) { fZNormal = z; }

void STLinearTrack::AddHit(STHit *hit)
{
  TVector3 position = hit -> GetPosition();
  fXCentroid = (fXCentroid * fChargeSum  +  hit -> GetCharge() * position.X());
  fYCentroid = (fYCentroid * fChargeSum  +  hit -> GetCharge() * position.Y());
  fZCentroid = (fZCentroid * fChargeSum  +  hit -> GetCharge() * position.Z());

  fChargeSum += hit -> GetCharge();

  fXCentroid *= 1./fChargeSum;
  fYCentroid *= 1./fChargeSum;
  fZCentroid *= 1./fChargeSum;

  fHitPointerArray -> push_back(hit);
  fHitIDArray -> push_back(hit -> GetHitID());

  fIsFitted = kFALSE;
}

   Int_t STLinearTrack::GetTrackID()   const { return fTrackID; }
  Bool_t STLinearTrack::IsPrimary()    const { return fIsPrimary; }
  Bool_t STLinearTrack::IsFitted()     const { return fIsFitted; }

TVector3 STLinearTrack::GetVertex()    const { return TVector3(fXVertex, fYVertex, fZVertex); }
TVector3 STLinearTrack::GetNormal()    const { return TVector3(fXNormal, fYNormal, fZNormal); }
TVector3 STLinearTrack::GetCentroid()  const { return TVector3(fXCentroid, fYCentroid, fZCentroid); }

Double_t STLinearTrack::GetXVertex()   const { return fXVertex; }
Double_t STLinearTrack::GetYVertex()   const { return fYVertex; }
Double_t STLinearTrack::GetZVertex()   const { return fZVertex; }

Double_t STLinearTrack::GetXNormal()   const { return fXNormal; }
Double_t STLinearTrack::GetYNormal()   const { return fYNormal; }
Double_t STLinearTrack::GetZNormal()   const { return fZNormal; }

Double_t STLinearTrack::GetXCentroid() const { return fXCentroid; }
Double_t STLinearTrack::GetYCentroid() const { return fYCentroid; }
Double_t STLinearTrack::GetZCentroid() const { return fZCentroid; }

Double_t STLinearTrack::GetChargeSum() const { return fChargeSum; }
   Int_t STLinearTrack::GetNumHits()         { return fHitIDArray -> size(); }

std::vector<Int_t>  *STLinearTrack::GetHitIDArray()      { return fHitIDArray; }
std::vector<STHit*> *STLinearTrack::GetHitPointerArray() { return fHitPointerArray; }

Int_t  STLinearTrack::GetHitID(Int_t i) { return fHitIDArray -> at(i); }
STHit* STLinearTrack::GetHit(Int_t i)   { return fHitPointerArray -> at(i); }
