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
    void SetPosition(TVector3 vec);                          ///< Position setter
    void SetPosition(Double_t x, Double_t y, Double_t z);    ///< Position setter
    void SetPosSigma(TVector3 vec);                          ///< Position sigma setter
    void SetPosSigma(Double_t dx, Double_t dy, Double_t dz); ///< Position sigma setter
    void SetCharge(Double_t charge);                         ///< Charge setter

      Bool_t IsClustered()  const; ///< Clustered flag getter
       Int_t GetHitID()     const; ///< Hit ID getter
       Int_t GetClusterID() const; ///< Cluster ID getter
       Int_t GetTrackID()   const; ///< Track ID getter
    TVector3 GetPosition()  const; ///< Position getter
    TVector3 GetPosSigma()  const; ///< Position sigma getter
    Double_t GetCharge()    const; ///< Charge getter

  protected:
      Bool_t fIsClustered; ///< Clustered flag
       Int_t fHitID;       ///< Hit ID
       Int_t fClusterID;   ///< Cluster ID having this hit
       Int_t fTrackID;     ///< Track ID having this hit
    TVector3 fPosition;    ///< Position
    TVector3 fPosSigma;    ///< Position error
    Double_t fCharge;      ///< Charge

  ClassDef(STHit, 3);
};

#endif
