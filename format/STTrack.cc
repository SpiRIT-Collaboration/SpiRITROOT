/**
 * @brief STTrack Class
 *
 * @author JungWoo Lee
 */

#include "STTrack.hh"

ClassImp(STTrack)

STTrack::STTrack()
{
  // Units: mm, MeV, e(charge)
  fParentID = -1;
  fTrackID = -1;
  fRiemannID = -1;
  
  fPID = -1;
  fMass = -1;
  
  fVx = 0;
  fVy = 0;
  fVz = 0;

  fPx = 0;
  fPy = 0;
  fPz = 0;

  fCharge = 0;
  fChi2 = -1;
  fNDF = -1;
}

void STTrack::SetParentID(Int_t value)                           { fParentID = value; }
void STTrack::SetTrackID(Int_t value)                            { fTrackID = value; }
void STTrack::SetRiemannID(Int_t value)                          { fRiemannID = value; }

void STTrack::SetPID(Int_t value)                                { fPID = value; }
void STTrack::SetMass(Double_t value)                            { fMass = value; }

void STTrack::SetVertex(TVector3 vector)                         { fVx = vector.X(); fVy = vector.Y(); fVz = vector.Z(); }
void STTrack::SetVertex(Double_t x, Double_t y, Double_t z)      { fVx = x; fVy = y; fVz = z; }

void STTrack::SetMomentum(TVector3 vector)                       { fPx = vector.X(); fPy = vector.Y(); fPz = vector.Z(); }
void STTrack::SetMomentum(Double_t px, Double_t py, Double_t pz) { fPx = px; fPy = py; fPz = pz; }

void STTrack::SetCharge(Int_t value)                             { fCharge = value; }
void STTrack::SetTrackLength(Double_t value)                     { fTrackLength = value; }
void STTrack::AdddEdx(Double_t value)                            { fdEdx.push_back(value); }

void STTrack::SetChi2(Double_t value)                            { fChi2 = value; }
void STTrack::SetNDF(Int_t value)                                { fNDF = value; }

void STTrack::AddHitID(Double_t value)                            { fHitIDArray.push_back(value); }

Int_t STTrack::GetParentID()                    { return fParentID; }
Int_t STTrack::GetTrackID()                     { return fTrackID; }
Int_t STTrack::GetRiemannID()                   { return fRiemannID; }

Int_t STTrack::GetPID()                         { return fPID; }
Double_t STTrack::GetMass()                     { return fMass; }

TVector3 STTrack::GetVertex()                   { return TVector3(fVx, fVy, fVz); }
Double_t STTrack::GetVx()                       { return fVx; }
Double_t STTrack::GetVy()                       { return fVy; }
Double_t STTrack::GetVz()                       { return fVz; }

TVector3 STTrack::GetMomentum()                 { return TVector3(fPx, fPy, fPz); }
Double_t STTrack::GetP()                        { return GetMomentum().Mag(); }
Double_t STTrack::GetPt()                       { return GetMomentum().Perp(); }
Double_t STTrack::GetPx()                       { return fPx; }
Double_t STTrack::GetPy()                       { return fPy; }
Double_t STTrack::GetPz()                       { return fPz; }

Int_t STTrack::GetCharge()                      { return fCharge; }
Double_t STTrack::GetTrackLength()              { return fTrackLength; }
std::vector<Double_t> *STTrack::GetdEdxArray()  { return &fdEdx; }

Double_t STTrack::GetChi2()                     { return fChi2; }
Int_t STTrack::GetNDF()                         { return fNDF; }

std::vector<Double_t> *STTrack::GetHitIDArray() { return &fHitIDArray; }
