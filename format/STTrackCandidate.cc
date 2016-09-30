#include "STTrackCandidate.hh"
#include <iostream>

ClassImp(STTrackCandidate)

STTrackCandidate::STTrackCandidate()
{
  Clear();
}

void STTrackCandidate::Clear(Option_t *option)
{
  // Units: mm, MeV, e(charge)
  fPID = -99999;
  fMass = -99999;
  fCharge = -99999;

  fVx = -99999;
  fVy = -99999;
  fVz = -99999;

  fBeamVx = -99999;
  fBeamVy = -99999;
  fBeamVz = -99999;

  fBeamMomx = -99999;
  fBeamMomy = -99999;
  fBeamMomz = -99999;

  fKyotoLx = -99999;
  fKyotoLy = -99999;
  fKyotoLz = -99999;
  fKyotoRx = -99999;
  fKyotoRy = -99999;
  fKyotoRz = -99999;
  fKatanax = -99999;
  fKatanay = -99999;
  fKatanaz = -99999;

  fPx = -99999;
  fPy = -99999;
  fPz = -99999;

  fTrackLength = -99999;
  fdEdxTotal = -99999;

  fRiemanndEdx = -99999;
  fChi2 = -99999;
  fNDF = -99999;

  fPVal = -99999;

  fdEdxArray.clear();
}

void STTrackCandidate::SetPID(Int_t value)                                { fPID = value; }
void STTrackCandidate::SetMass(Double_t value)                            { fMass = value; }
void STTrackCandidate::SetCharge(Int_t value)                             { fCharge = value; }

void STTrackCandidate::SetVertex(TVector3 vector)                         { fVx = vector.X(); fVy = vector.Y(); fVz = vector.Z(); }
void STTrackCandidate::SetVertex(Double_t x, Double_t y, Double_t z)      { fVx = x; fVy = y; fVz = z; }

void STTrackCandidate::SetBeamVertex(TVector3 vector)                     { fBeamVx = vector.X(); fBeamVy = vector.Y(); fBeamVz = vector.Z(); }
void STTrackCandidate::SetBeamVertex(Double_t x, Double_t y, Double_t z)  { fBeamVx = x; fBeamVy = y; fBeamVz = z; }
void STTrackCandidate::SetBeamMomentum(TVector3 vector)                   { fBeamMomx = vector.X(); fBeamMomy = vector.Y(); fBeamMomz = vector.Z(); }
void STTrackCandidate::SetBeamMomentum(Double_t x, Double_t y, Double_t z)  { fBeamMomx = x; fBeamMomy = y; fBeamMomz = z; }

void STTrackCandidate::SetKyotoLHit(TVector3 vector)                         { fKyotoLx = vector.X(); fKyotoLy = vector.Y(); fKyotoLz = vector.Z(); }
void STTrackCandidate::SetKyotoRHit(TVector3 vector)                         { fKyotoRx = vector.X(); fKyotoRy = vector.Y(); fKyotoRz = vector.Z(); }
void STTrackCandidate::SetKatanaHit(TVector3 vector)                         { fKatanax = vector.X(); fKatanay = vector.Y(); fKatanaz = vector.Z(); }

void STTrackCandidate::SetMomentum(TVector3 vector)                       { fPx = vector.X(); fPy = vector.Y(); fPz = vector.Z(); }
void STTrackCandidate::SetMomentum(Double_t px, Double_t py, Double_t pz) { fPx = px; fPy = py; fPz = pz; }

void STTrackCandidate::AdddEdx(Double_t value)                            { fdEdxArray.push_back(value); }
void STTrackCandidate::SetTrackLength(Double_t value)                     { fTrackLength = value; }
void STTrackCandidate::SetTotaldEdx(Double_t value)                       { fdEdxTotal = value; }

void STTrackCandidate::SetRiemanndEdx(Double_t value)                     { fRiemanndEdx = value; }
void STTrackCandidate::SetChi2(Double_t value)                            { fChi2 = value; }
void STTrackCandidate::SetNDF(Int_t value)                                { fNDF = value; }

void STTrackCandidate::SetPVal(Double_t value)                            { fPVal = value; }

void STTrackCandidate::SetCovSeed(TMatrixDSym covSeed)                    { fCovSeed.ResizeTo(covSeed); fCovSeed = covSeed; }


Int_t STTrackCandidate::GetPID()                         { return fPID; }
Double_t STTrackCandidate::GetMass()                     { return fMass; }
Int_t STTrackCandidate::GetCharge()                      { return fCharge; }

Double_t STTrackCandidate::DistanceToPrimaryVertex(TVector3 vertex) { return (GetVertex() - vertex).Mag(); }
TVector3 STTrackCandidate::GetVertex()                   { return TVector3(fVx, fVy, fVz); }
TVector3 STTrackCandidate::GetTargetPlaneVertex()        { return TVector3(fBeamVx, fBeamVy, fBeamVz); }
TVector3 STTrackCandidate::GetBeamVertex()               { return TVector3(fBeamVx, fBeamVy, fBeamVz); }
Double_t STTrackCandidate::GetVx()                       { return fVx; }
Double_t STTrackCandidate::GetVy()                       { return fVy; }
Double_t STTrackCandidate::GetVz()                       { return fVz; }
Double_t STTrackCandidate::GetBeamVx()                   { return fBeamVx; }
Double_t STTrackCandidate::GetBeamVy()                   { return fBeamVy; }
Double_t STTrackCandidate::GetBeamVz()                   { return fBeamVz; }

TVector3 STTrackCandidate::GetTargetPlaneMomentum()      { return TVector3(fBeamMomx, fBeamMomy, fBeamMomz); }
TVector3 STTrackCandidate::GetBeamMomentum()             { return TVector3(fBeamMomx, fBeamMomy, fBeamMomz); }
Double_t STTrackCandidate::GetBeamMomx()                 { return fBeamMomx; }
Double_t STTrackCandidate::GetBeamMomy()                 { return fBeamMomy; }
Double_t STTrackCandidate::GetBeamMomz()                 { return fBeamMomz; }

TVector3 STTrackCandidate::GetKyotoLHit()                { return TVector3(fKyotoLx, fKyotoLy, fKyotoLz); }
Double_t STTrackCandidate::GetKyotoLHitX()               { return fKyotoLx; }
Double_t STTrackCandidate::GetKyotoLHitY()               { return fKyotoLy; }
Double_t STTrackCandidate::GetKyotoLHitZ()               { return fKyotoLz; }

TVector3 STTrackCandidate::GetKyotoRHit()                { return TVector3(fKyotoRx, fKyotoRy, fKyotoRz); }
Double_t STTrackCandidate::GetKyotoRHitX()               { return fKyotoRx; }
Double_t STTrackCandidate::GetKyotoRHitY()               { return fKyotoRy; }
Double_t STTrackCandidate::GetKyotoRHitZ()               { return fKyotoRz; }

TVector3 STTrackCandidate::GetKatanaHit()                { return TVector3(fKatanax, fKatanay, fKatanaz); }
Double_t STTrackCandidate::GetKatanaHitX()               { return fKatanax; }
Double_t STTrackCandidate::GetKatanaHitY()               { return fKatanay; }
Double_t STTrackCandidate::GetKatanaHitZ()               { return fKatanaz; }

TVector3 STTrackCandidate::GetMomentum() const           { return TVector3(fPx, fPy, fPz); }
Double_t STTrackCandidate::GetP() const                  { return GetMomentum().Mag(); }
Double_t STTrackCandidate::GetPt()                       { return GetMomentum().Perp(); }
Double_t STTrackCandidate::GetPx()                       { return fPx; }
Double_t STTrackCandidate::GetPy()                       { return fPy; }
Double_t STTrackCandidate::GetPz()                       { return fPz; }

Double_t STTrackCandidate::GetTrackLength()              { return fTrackLength; }
std::vector<Double_t> *STTrackCandidate::GetdEdxArray()  { return &fdEdxArray; }

Double_t STTrackCandidate::GetTotaldEdx()                { return fdEdxTotal; }

Double_t STTrackCandidate::GetRiemanndEdx()              { return fRiemanndEdx; }
Double_t STTrackCandidate::GetChi2()                     { return fChi2; }
Int_t STTrackCandidate::GetNDF()                         { return fNDF; }

Double_t STTrackCandidate::GetPVal()                     { return fPVal; }

Int_t STTrackCandidate::GetdEdxWithCut(Double_t &dEdx, Int_t &numUsedPoints, Double_t lowCut, Double_t highCut, Int_t numCut)
{
  numUsedPoints = fdEdxArray.size();

  sort(fdEdxArray.begin(), fdEdxArray.end());

  Int_t idxLow = Int_t(numUsedPoints * lowCut);
  Int_t idxHigh = Int_t(numUsedPoints * highCut);

  numUsedPoints = idxHigh - idxLow;
  if (numUsedPoints < numCut)
    return -1;

  dEdx = 0;
  for (Int_t idEdx = idxLow; idEdx < idxHigh; idEdx++)
    dEdx += fdEdxArray[idEdx];
  dEdx = dEdx/numUsedPoints;

  return 2;
}

Double_t STTrackCandidate::GetdEdxWithCut(Double_t lowCut, Double_t highCut)
{
  Int_t numPoints;
  Double_t dEdx;

  if (GetdEdxWithCut(dEdx, numPoints, lowCut, highCut, 1) == 2)
    return dEdx;

  return -1;
}

void STTrackCandidate::SetTrackCandidate(STTrackCandidate *track)
{
  fPID = track -> GetPID();
  fMass = track -> GetMass();
  fCharge = track -> GetCharge();

  SetVertex(track -> GetVertex());
  SetBeamVertex(track -> GetBeamVertex());
  SetBeamMomentum(track -> GetBeamMomentum());
  SetKyotoLHit(track -> GetKyotoLHit());
  SetKyotoRHit(track -> GetKyotoRHit());
  SetKatanaHit(track -> GetKatanaHit());
  SetMomentum(track -> GetMomentum());

  fTrackLength = track -> GetTrackLength();
  fdEdxTotal = track -> GetTotaldEdx();
  fRiemanndEdx = track -> GetRiemanndEdx();
  fChi2 = track -> GetChi2();
  fNDF = track -> GetNDF();
  fPVal = track -> GetPVal();

  fdEdxArray.clear();
  std::vector<Double_t> *tempArray = track -> GetdEdxArray();
  Int_t n = tempArray -> size();
  for (Int_t i = 0; i < n; i++)
    fdEdxArray.push_back(tempArray -> at(i));

  SetCovSeed(track -> GetCovSeed());
}

const TMatrixDSym& STTrackCandidate::GetCovSeed() const { return fCovSeed; }
