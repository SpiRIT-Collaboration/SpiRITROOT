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

class STHit : public TObject 
{
  public:
    STHit();
    STHit(STHit *hit);
    STHit(Int_t hitID, TVector3 vec, Double_t charge);
    STHit(Int_t hitID, Double_t x, Double_t y, Double_t z, Double_t charge);
    ~STHit();

    /// Hit setter
    void SetHit(Int_t hitID, TVector3 vec, Double_t charge);
    /// Hit setter
    void SetHit(Int_t hitID, Double_t x, Double_t y, Double_t z, Double_t charge);

    void SetIsClustered(Bool_t value = kTRUE);               ///< Clustered flag setter
    void SetHitID(Int_t hitID);                              ///< Hit ID setter
    void SetClusterID(Int_t clusterID);                      ///< Cluster stter
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

      Bool_t IsClustered()  const; ///< Clustered flag getter
       Int_t GetHitID()     const; ///< Hit ID getter
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

  private:
    void Init();

  ClassDef(STHit, 4);
};

class STHitSortTheta {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Theta() > hit2 -> GetPosition().Theta(); }
};

class STHitSortThetaInv {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Theta() < hit2 -> GetPosition().Theta(); }
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

#endif
