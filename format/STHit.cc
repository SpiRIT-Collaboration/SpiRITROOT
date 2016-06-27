/**
 *  @brief STHit Class
 *
 *  @author Genie Jhang ( geniejhang@majimak.com )
 *  @author JungWoo Lee
 */

#include "STHit.hh"

STHit::STHit()
{
  Clear();
}

STHit::STHit(Int_t hitID, TVector3 vec, Double_t charge)
{
  Clear();

  SetHit(hitID, vec, charge);
}

STHit::STHit(Int_t hitID, Double_t x, Double_t y, Double_t z, Double_t charge)
{
  Clear();

  SetHit(hitID, x, y, z, charge);
}

STHit::STHit(STHit *hit)
{
  Clear();

  fIsClustered = hit -> IsClustered();
  fClusterID = hit -> GetClusterID();
  fTrackID = hit -> GetTrackID();

  SetHit(hit -> GetHitID(), hit -> GetPosition(), hit -> GetCharge());

  fDx = hit -> GetDx();
  fDy = hit -> GetDy();
  fDz = hit -> GetDz();

  fRow = hit -> GetRow();
  fLayer = hit -> GetLayer();
  fTb = hit -> GetTb();

  fChi2 = hit -> GetChi2();
  fNDF = hit -> GetNDF();
}

void STHit::Clear(Option_t *)
{
  fIsClustered = kFALSE;
  fHitID = -1;
  fTrackID = -1;
  fClusterID = -1;

  SetHit(-1, 0, 0, -1000, -1);

  fDx = 0.8;
  fDy = 0.22;
  fDz = 1.2;

  fRow = -1;
  fLayer = -1;
  fTb = -1;

  fChi2 = -1;
  fNDF = 0;
}

void STHit::SetHit(Int_t hitID, TVector3 vec, Double_t charge) 
{ 
   fHitID = hitID; 
       fX = vec.X();
       fY = vec.Y();
       fZ = vec.Z();
  fCharge = charge; 
}

void STHit::SetHit(Int_t hitID, Double_t x, Double_t y, Double_t z, Double_t charge) 
{ 
   fHitID = hitID; 
       fX = x;
       fY = y;
       fZ = z;
  fCharge = charge; 
}

void STHit::SetIsClustered(Bool_t value)                        { fIsClustered = value; }
void STHit::SetHitID(Int_t hitID)                               { fHitID = hitID; }
void STHit::SetClusterID(Int_t clusterID)                       { fClusterID = clusterID; fIsClustered = kTRUE; }
void STHit::SetTrackID(Int_t trackID)                           { fTrackID = trackID; }
void STHit::SetX(Double_t x)                                    { fX = x; }
void STHit::SetY(Double_t y)                                    { fY = y; }
void STHit::SetZ(Double_t z)                                    { fZ = z; }
void STHit::SetPosition(TVector3 vec)                           { fX = vec.X(); fY = vec.Y(); fZ = vec.Z(); }
void STHit::SetPosition(Double_t x, Double_t y, Double_t z)     { fX = x; fY = y; fZ = z; }
void STHit::SetDx(Double_t dx)                                  { fDx = dx; }
void STHit::SetDy(Double_t dy)                                  { fDy = dy; }
void STHit::SetDz(Double_t dz)                                  { fDz = dz; }
void STHit::SetPosSigma(TVector3 vec)                           { fDx = vec.X(); fDy = vec.Y(); fDz = vec.Z(); }
void STHit::SetPosSigma(Double_t dx, Double_t dy, Double_t dz)  { fDx = dx; fDy = dy; fDz = dz; }
void STHit::SetCharge(Double_t charge)                          { fCharge = charge; }

void STHit::SetRow(Int_t row)                                   { fRow = row; }
void STHit::SetLayer(Int_t layer)                               { fLayer = layer; }
void STHit::SetTb(Double_t tb)                                  { fTb = tb; }

void STHit::SetChi2(Double_t chi2)                              { fChi2 = chi2; }
void STHit::SetNDF(Int_t ndf)                                   { fNDF = ndf; }

void STHit::AddTrackCand(Int_t trackID)                         { fTrackCandArray.push_back(trackID); }
void STHit::RemoveTrackCand(Int_t trackID)
{
  for (auto i = 0; i < fTrackCandArray.size(); i++) {
    if (fTrackCandArray[i] == trackID) {
      fTrackCandArray.erase(fTrackCandArray.begin()+i); 
      return;
    }
  }
  fTrackCandArray.push_back(-1);
}

   Int_t STHit::GetHitID()     const   { return fHitID; }
   Int_t STHit::GetClusterID() const   { return (fIsClustered ? fClusterID : -1); }
   Int_t STHit::GetTrackID()   const   { return fTrackID; }
Double_t STHit::GetX()         const   { return fX; }
Double_t STHit::GetY()         const   { return fY; }
Double_t STHit::GetZ()         const   { return fZ; }
TVector3 STHit::GetPosition()  const   { return TVector3(fX, fY, fZ); }
Double_t STHit::GetDx()        const   { return fDx; }
Double_t STHit::GetDy()        const   { return fDy; }
Double_t STHit::GetDz()        const   { return fDz; }
TVector3 STHit::GetPosSigma()  const   { return TVector3(fDx, fDy, fDz); }
Double_t STHit::GetCharge()    const   { return fCharge; }
  Bool_t STHit::IsClustered()  const   { return fIsClustered; }

   Int_t STHit::GetRow()       const   { return fRow; }
   Int_t STHit::GetLayer()     const   { return fLayer; }
Double_t STHit::GetTb()        const   { return fTb; }

Double_t STHit::GetChi2()      const   { return fChi2; }
   Int_t STHit::GetNDF()       const   { return fNDF; }

Int_t STHit::GetNumTrackCands()  { return fTrackCandArray.size(); }
std::vector<Int_t> *STHit::GetTrackCandArray()  { return &fTrackCandArray; }
