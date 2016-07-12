/**
 *  @brief STHit Class
 *
 *  @author Genie Jhang ( geniejhang@majimak.com )
 *  @author JungWoo Lee
 *
 *  @detail container for a hit data
 */

#ifndef STHIT_HH
#define STHIT_HH

#include "TObject.h"
#include "TVector3.h"
#include "TMath.h"

#include <vector>

class STHit : public TObject 
{
  public:
    STHit();
    STHit(STHit *hit);
    STHit(Int_t hitID, TVector3 vec, Double_t charge);
    STHit(Int_t hitID, Double_t x, Double_t y, Double_t z, Double_t charge);
    virtual ~STHit() {}

    /// Hit setter
    void SetHit(Int_t hitID, TVector3 vec, Double_t charge);
    /// Hit setter
    void SetHit(Int_t hitID, Double_t x, Double_t y, Double_t z, Double_t charge);

    void Clear(Option_t * = "");                             ///< Clear method for reuse object

    void SetIsClustered(Bool_t value = kTRUE);               ///< Clustered flag setter
    void SetHitID(Int_t hitID);                              ///< Hit ID setter
    virtual void SetClusterID(Int_t clusterID);              ///< Cluster stter
    void SetTrackID(Int_t trackID);                          ///< Track ID setter
    void SetX(Double_t x);                                   ///< x position setter
    void SetY(Double_t y);                                   ///< y position setter
    void SetZ(Double_t z);                                   ///< z position setter
    void SetPosition(TVector3 vec);                          ///< Position setter
    void SetPosition(Double_t x, Double_t y, Double_t z);    ///< Position setter
    void SetDx(Double_t dx);                                 ///< x position sigma setter
    void SetDy(Double_t dy);                                 ///< y position sigma setter
    void SetDz(Double_t dz);                                 ///< z position sigma setter
    void SetPosSigma(TVector3 vec);                          ///< Position sigma setter
    void SetPosSigma(Double_t dx, Double_t dy, Double_t dz); ///< Position sigma setter
    void SetCharge(Double_t charge);                         ///< Charge setter

    void SetRow(Int_t row);                                  ///< Row number setter
    void SetLayer(Int_t layer);                              ///< Layer number setter
    void SetTb(Double_t tb);                                 ///< Time bucket number setter

    void SetChi2(Double_t chi2);                             ///< Chi-square setter
    void SetNDF(Int_t ndf);                                  ///< NDF setter

    void AddTrackCand(Int_t trackID);
    void RemoveTrackCand(Int_t trackID);

    virtual Bool_t IsClustered() const; ///< Clustered flag getter
    virtual  Int_t GetHitID()    const; ///< Hit ID getter

       Int_t GetClusterID() const; ///< Cluster ID getter
       Int_t GetTrackID()   const; ///< Track ID getter
    Double_t GetX()         const; ///< x position getter
    Double_t GetY()         const; ///< y position getter
    Double_t GetZ()         const; ///< z position getter
    TVector3 GetPosition()  const; ///< Position getter
    TVector3 GetPosSigma()  const; ///< Position sigma getter
    Double_t GetDx()        const; ///< x position sigma getter
    Double_t GetDy()        const; ///< y position sigma getter
    Double_t GetDz()        const; ///< z position sigma getter
    Double_t GetCharge()    const; ///< Charge getter

       Int_t GetRow()       const; ///< Row number getter
       Int_t GetLayer()     const; ///< Layer number sgtter
    Double_t GetTb()        const; ///< Time bucket number getter

    Double_t GetChi2()      const; ///< Chi-square getter
       Int_t GetNDF()       const; ///< NDF getter

    Int_t GetNumTrackCands();
    std::vector<Int_t> *GetTrackCandArray();

  protected:
      Bool_t fIsClustered; ///< Clustered flag
       Int_t fHitID;       ///< Hit ID
       Int_t fClusterID;   ///< Cluster ID having this hit
       Int_t fTrackID;     ///< Track ID having this hit
    Double_t fX;           ///< x position
    Double_t fY;           ///< y position
    Double_t fZ;           ///< z position
    Double_t fDx;          ///< x position sigma
    Double_t fDy;          ///< y position sigma
    Double_t fDz;          ///< z position sigma
    Double_t fCharge;      ///< Charge

       Int_t fRow;         ///< Row number of the pad having maximum peak
       Int_t fLayer;       ///< Layer number of the pad having maximum peak
    Double_t fTb;          ///< Time bucket number of hit position

    Double_t fChi2;        ///< Chi-square of hit time fit
       Int_t fNDF;         ///< NDF of hit time fit

    std::vector<Int_t> fTrackCandArray; //! <

  ClassDef(STHit, 7);
};

class STHitSortDirection
{
  public:
    STHitSortDirection(TVector3 p):fP(p) {}

    Bool_t operator() (STHit* hit1, STHit* hit2) 
    { return hit1 -> GetPosition().Dot(fP) > hit2 -> GetPosition().Dot(fP); }

  private:
    TVector3 fP;
};

class STHitSortThetaFromP 
{
  public:
    STHitSortThetaFromP(TVector3 p):fP(p) {}

    Bool_t operator() (STHit* hit1, STHit* hit2) {
      fP1 = hit1 -> GetPosition() - fP;
      fP2 = hit2 -> GetPosition() - fP;
      return TMath::ATan2(fP1.Z(),fP1.X()) > TMath::ATan2(fP2.Z(),fP2.X());
    }

  private:
    TVector3 fP;
    TVector3 fP1;
    TVector3 fP2;
};

class STHitSortTheta {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2) { 
      Double_t t1 = TMath::ATan2(hit1 -> GetPosition().Z(), hit1 -> GetPosition().X());
      Double_t t2 = TMath::ATan2(hit2 -> GetPosition().Z(), hit2 -> GetPosition().X());
      return t1 > t2;
    }
};

class STHitSortThetaInv {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2) {
      Double_t t1 = TMath::ATan2(hit1 -> GetPosition().Z(), hit1 -> GetPosition().X());
      Double_t t2 = TMath::ATan2(hit2 -> GetPosition().Z(), hit2 -> GetPosition().X());
      return t1 < t2;
    }
};

class STHitSortR {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Mag() > hit2 -> GetPosition().Mag(); }
};

class STHitSortRInv {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Mag() < hit2 -> GetPosition().Mag(); }
};

class STHitSortRInvFromP {
  public:
    STHitSortRInvFromP(TVector3 p):fP(p) {}
    Bool_t operator() (STHit* hit1, STHit* hit2){
      fP1 = hit1 -> GetPosition() - fP;
      fP2 = hit2 -> GetPosition() - fP;
      if (fP1.Mag() == fP2.Mag()) {
        if (fP1.X() == fP2.X()) {
          if (fP1.Y() == fP2.Y()) {
            return fP1.Z() > fP2.Z();
          } return fP1.Y() > fP2.Z();
        } return fP1.X() > fP2.X();
      } return fP1.Mag() < fP2.Mag(); 
    }

  private:
    TVector3 fP;
    TVector3 fP1;
    TVector3 fP2;
};

class STHitSortZ {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Z() > hit2 -> GetPosition().Z(); }
};

class STHitSortZInv {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Z() < hit2 -> GetPosition().Z(); }
};

class STHitSortX {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().X() > hit2 -> GetPosition().X(); }
};

class STHitSortXInv {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().X() < hit2 -> GetPosition().X(); }
};

class STHitSortY {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Y() > hit2 -> GetPosition().Y(); }
};

class STHitSortYInv {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Y() < hit2 -> GetPosition().Y(); }
};

class STHitSortCharge {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2) {
      if (hit1 -> GetCharge() == hit2 -> GetCharge())
        return hit1 -> GetY() > hit2 -> GetY();
      return hit1 -> GetCharge() > hit2 -> GetCharge();
    }
};

class STHitSortChargeInv {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetCharge() < hit2 -> GetCharge(); }
};

class STHitSortXYZInv
{
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { 
      if (hit1 -> GetPosition().Z() == hit2 -> GetPosition().Z()) {
        if (hit1 -> GetPosition().X() == hit2 -> GetPosition().X())
          return hit1 -> GetPosition().Y() < hit2 -> GetPosition().Y(); 
        else 
          return hit1 -> GetPosition().X() < hit2 -> GetPosition().X(); 
      }
      return hit1 -> GetPosition().Z() < hit2 -> GetPosition().Z(); 
    }
};

class STHitSortRho {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return (hit1 -> GetPosition().X() * hit1 -> GetPosition().X() + hit1 -> GetPosition().Z() * hit1 -> GetPosition().Z())
      > (hit2 -> GetPosition().X() * hit2 -> GetPosition().X() + hit2 -> GetPosition().Z() * hit2 -> GetPosition().Z()); }
};

class STHitSortRhoInv {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return (hit1 -> GetPosition().X() * hit1 -> GetPosition().X() + hit1 -> GetPosition().Z() * hit1 -> GetPosition().Z())
      < (hit2 -> GetPosition().X() * hit2 -> GetPosition().X() + hit2 -> GetPosition().Z() * hit2 -> GetPosition().Z()); }
};

class STHitSortLayer {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetLayer() > hit2 -> GetLayer(); }
};

class STHitSortRow {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetRow() > hit2 -> GetRow(); }
};

#endif
