/**
x * @brief STTrack Class
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

  fIsFitted = kFALSE;
  
  fPID = -1;
  fMass = -1;
  fCharge = 0;
  
  fVx = 0;
  fVy = 0;
  fVz = 0;

  fBeamVx = 0;
  fBeamVy = 0;
  fBeamVz = 0;

  fPx = 0;
  fPy = 0;
  fPz = 0;

  fTrackLength = -1;
  fdEdxTotal = -1;

  fChi2 = -1;
  fNDF = -1;
}

void STTrack::SetParentID(Int_t value)                           { fParentID = value; }
void STTrack::SetTrackID(Int_t value)                            { fTrackID = value; }
void STTrack::SetRiemannID(Int_t value)                          { fRiemannID = value; }

void STTrack::SetIsFitted(Bool_t value)                          { fIsFitted = value; }

void STTrack::SetPID(Int_t value)                                { fPID = value; }
void STTrack::SetMass(Double_t value)                            { fMass = value; }
void STTrack::SetCharge(Int_t value)                             { fCharge = value; }

void STTrack::SetVertex(TVector3 vector)                         { fVx = vector.X(); fVy = vector.Y(); fVz = vector.Z(); }
void STTrack::SetVertex(Double_t x, Double_t y, Double_t z)      { fVx = x; fVy = y; fVz = z; }

void STTrack::SetBeamVertex(TVector3 vector)                         { fBeamVx = vector.X(); fBeamVy = vector.Y(); fBeamVz = vector.Z(); }
void STTrack::SetBeamVertex(Double_t x, Double_t y, Double_t z)      { fBeamVx = x; fBeamVy = y; fBeamVz = z; }

void STTrack::SetMomentum(TVector3 vector)                       { fPx = vector.X(); fPy = vector.Y(); fPz = vector.Z(); }
void STTrack::SetMomentum(Double_t px, Double_t py, Double_t pz) { fPx = px; fPy = py; fPz = pz; }

void STTrack::AdddEdx(Double_t value)                            { fdEdxArray.push_back(value); }

void STTrack::SetTrackLength(Double_t value)                     { fTrackLength = value; }
void STTrack::SetTotaldEdx(Double_t value)                       { fdEdxTotal = value; }

void STTrack::SetChi2(Double_t value)                            { fChi2 = value; }
void STTrack::SetNDF(Int_t value)                                { fNDF = value; }

void STTrack::AddHitID(Int_t value)                              { fHitIDArray.push_back(value); }

void STTrack::AddTrackCandidate(STTrackCandidate *track)         { fTrackCandidateArray.push_back(track); }

Int_t STTrack::GetParentID()                    { return fParentID; }
Int_t STTrack::GetTrackID()                     { return fTrackID; }
Int_t STTrack::GetRiemannID()                   { return fRiemannID; }

Bool_t STTrack::IsFitted()                      { return fIsFitted; }

Int_t STTrack::GetPID()                         { return fPID; }
Double_t STTrack::GetMass()                     { return fMass; }
Int_t STTrack::GetCharge()                      { return fCharge; }

TVector3 STTrack::GetVertex()                   { return TVector3(fVx, fVy, fVz); }
TVector3 STTrack::GetBeamVertex()               { return TVector3(fBeamVx, fBeamVy, fBeamVz); }

Double_t STTrack::GetVx()                       { return fVx; }
Double_t STTrack::GetVy()                       { return fVy; }
Double_t STTrack::GetVz()                       { return fVz; }
Double_t STTrack::GetBeamVx()                   { return fBeamVx; }
Double_t STTrack::GetBeamVy()                   { return fBeamVy; }
Double_t STTrack::GetBeamVz()                   { return fBeamVz; }

TVector3 STTrack::GetMomentum()                 { return TVector3(fPx, fPy, fPz); }
Double_t STTrack::GetP()                        { return GetMomentum().Mag(); }
Double_t STTrack::GetPt()                       { return GetMomentum().Perp(); }
Double_t STTrack::GetPx()                       { return fPx; }
Double_t STTrack::GetPy()                       { return fPy; }
Double_t STTrack::GetPz()                       { return fPz; }

Double_t STTrack::GetTrackLength()              { return fTrackLength; }
std::vector<Double_t> *STTrack::GetdEdxArray()  { return &fdEdxArray; }

Double_t STTrack::GetTotaldEdx()                { return fdEdxTotal; }

Double_t STTrack::GetChi2()                     { return fChi2; }
Int_t STTrack::GetNDF()                         { return fNDF; }

std::vector<Int_t> *STTrack::GetHitIDArray()    { return &fHitIDArray; }

std::vector<STTrackCandidate *> *STTrack::GetTrackCandidateArray() { return &fTrackCandidateArray; }

Int_t STTrack::GetNumTrackCandidates() { return fTrackCandidateArray.size(); }

STTrackCandidate *STTrack::GetTrackCandidate(Int_t idx)
{
  return fTrackCandidateArray.size() > idx ? fTrackCandidateArray[idx] : nullptr;
}

Bool_t STTrack::SelectTrackCandidate(Int_t idx)
{
  if (fTrackCandidateArray.size() < idx) 
    return kFALSE;

  SelectTrackCandidate(fTrackCandidateArray[idx]);
  return kTRUE;
}

void STTrack::SelectTrackCandidate(STTrackCandidate *track)
{
  fPID = track -> GetPID();
  fMass = track -> GetMass();
  fCharge = track -> GetCharge();

  SetVertex(track -> GetVertex());
  SetBeamVertex(track -> GetBeamVertex());
  SetMomentum(track -> GetMomentum());

  fTrackLength = track -> GetTrackLength();

  fTrackLength = track -> GetTotaldEdx();
  fChi2 = track -> GetChi2();
  fNDF = track -> GetNDF();

  fdEdxArray.clear();
  std::vector<Double_t> *tempArray = track -> GetdEdxArray();
  Int_t n = tempArray -> size();
  for (Int_t i = 0; i < n; i++)
    fdEdxArray.push_back(tempArray -> at(i));
}
