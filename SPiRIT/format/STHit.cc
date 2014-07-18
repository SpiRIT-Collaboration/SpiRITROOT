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
  fTrackID = -1;
  SetHit(-1, 0, 0, -1000, -1);

  fIsClustered = kFALSE;
  fClusterID = -1;
}

STHit::STHit(Int_t hitID, TVector3 vec, Double_t charge)
{
  fTrackID = -1;
  SetHit(hitID, vec, charge);

  fIsClustered = kFALSE;
  fClusterID = -1;
}

STHit::STHit(Int_t hitID, Double_t x, Double_t y, Double_t z, Double_t charge)
{
  fTrackID = -1;
  SetHit(hitID, x, y, z, charge);

  fIsClustered = kFALSE;
  fClusterID = -1;
}

STHit::~STHit()
{}

void STHit::SetTrackID(Int_t trackID)                                   { fTrackID = trackID; }
void STHit::SetHitID(Int_t hitID)                                       { fHitID = hitID; }
void STHit::SetHit(Int_t hitID, TVector3 vec, Double_t charge)                       { fHitID = hitID; fPosition = vec; fCharge = charge; }
void STHit::SetHit(Int_t hitID, Double_t x, Double_t y, Double_t z, Double_t charge) { fHitID = hitID; fPosition = TVector3(x, y, z); fCharge = charge; }

void STHit::SetPosition(TVector3 vec)                                   { fPosition = vec; }
void STHit::SetPosition(Double_t x, Double_t y, Double_t z)             { fPosition = TVector3(x, y, z); }
void STHit::SetPosSigma(TVector3 vec)                                   { fPositionSigma = vec; }
void STHit::SetPosSigma(Double_t dx, Double_t dy, Double_t dz)          { fPositionSigma = TVector3(dx, dy, dz); }
void STHit::SetCharge(Double_t charge)                                  { fCharge = charge; }

void STHit::SetIsClustered(Bool_t value)                                { fIsClustered = value; }
void STHit::SetClusterID(Int_t clusterID)                               { fClusterID = clusterID; fIsClustered = kTRUE; }

Int_t STHit::GetTrackID()                                               { return fTrackID; }
Int_t STHit::GetHitID()                                                 { return fHitID; }
TVector3 STHit::GetPosition()                                           { return fPosition; }
TVector3 STHit::GetPosSigma()                                           { return fPositionSigma; }
Double_t STHit::GetCharge()                                             { return fCharge; }
Bool_t STHit::GetIsClustered()                                          { return fIsClustered; }
Int_t STHit::GetClusterID()                                             { return (fIsClustered ? fClusterID : -1); }
