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
  fIsEmbed = hit -> IsEmbed();
  fIsSaturated = hit -> IsSaturated();  

  SetHit(hit -> GetHitID(), hit -> GetPosition(), hit -> GetCharge());

  fDx = hit -> GetDx();
  fDy = hit -> GetDy();
  fDz = hit -> GetDz();

  fCharge = hit -> GetCharge();

  fRow = hit -> GetRow();
  fLayer = hit -> GetLayer();
  fTb = hit -> GetTb();
  fSatTb = hit -> GetSaturatedTb();
    
  fChi2 = hit -> GetChi2();
  fNDF = hit -> GetNDF();

  fS = hit -> GetS();
}

void STHit::Clear(Option_t *)
{
  fIsClustered = kFALSE;
  fIsEmbed     = false;
  fHitID       = -1;
  fTrackID     = -1;
  fClusterID   = -1;
  fIsSaturated = false;
  fSatTb       = 9999;

  SetHit(-1, 0, 0, -1000, 0);

  fX = 0;
  fY = 0;
  fZ = 0;
  fDx = 0.8;
  fDy = 0.22;
  fDz = 1.2;

  fCharge = 0;

  fRow = -1;
  fLayer = -1;
  fTb = -1;

  fChi2 = -1;
  fNDF = 0;

  fS = 0;
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

void STHit::SetIsEmbed(Bool_t value)                            { fIsEmbed = value; }
void STHit::SetIsSaturated(Bool_t value)                        { fIsSaturated = value; }
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
void STHit::SetSaturatedTb(Double_t tb)                         { fSatTb = tb; }
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
  Bool_t STHit::IsEmbed()   const   {return fIsEmbed; }
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
  Bool_t STHit::IsSaturated()  const   { return fIsSaturated; }

   Int_t STHit::GetRow()       const   { return fRow; }
   Int_t STHit::GetLayer()     const   { return fLayer; }
Double_t STHit::GetTb()        const   { return fTb; }
Double_t STHit::GetSaturatedTb() const { return fSatTb; }

Double_t STHit::GetChi2()      const   { return fChi2; }
   Int_t STHit::GetNDF()       const   { return fNDF; }

Int_t STHit::GetNumTrackCands()  { return fTrackCandArray.size(); }
std::vector<Int_t> *STHit::GetTrackCandArray()  { return &fTrackCandArray; }

Int_t STHit::Compare(const TObject *obj) const
{
  auto s = ((STHit *) obj) -> GetS();
  if (fS - s < 0)
    return -1;
  else
    return 1;
}
