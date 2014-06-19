// =================================================
//  STHit Class
//
//  Description:
//    Container for a hit data
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 05. 19
// ================================================= 

#include "STHit.hh"

STHit::STHit()
{
  fPosition = -4;
  fCharge = -4;
}

STHit::~STHit()
{}

void STHit::SetTrackID(Int_t trackID)
{
  fTrackID = trackID;
}

void STHit::SetHit(TVector3 vec, Double_t charge)
{
  fPosition = vec;
  fCharge = charge;
}

void STHit::SetHit(Double_t x, Double_t y, Double_t z, Double_t charge)
{
  fPosition = TVector3(x, y, z);
  fCharge = charge;
}

void STHit::SetPosition(TVector3 vec)
{
  fPosition = vec;
}

void STHit::SetPosition(Double_t x, Double_t y, Double_t z)
{
  fPosition = TVector3(x, y, z);
}

void STHit::SetCharge(Double_t charge)
{
  fCharge = charge;
}

Int_t STHit::GetTrackID()
{
  return fTrackID;
}

TVector3 STHit::GetPosition()
{
  return fPosition;
}

Double_t STHit::GetCharge()
{
  return fCharge;
}
