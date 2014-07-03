// =================================================
//  STHit Class
//
//  Description:
//    Container for a hit data
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 05. 19
// ================================================= 

#ifndef _STHIT_H_
#define _STHIT_H_

#include "TROOT.h"
#include "TObject.h"
#include "TVector3.h"

/**
  * This class contains hit information
  * processed from STPad class data.
 **/

class STHit : public TObject {
  public:
    STHit();
    STHit(TVector3 vec, Double_t charge);
    STHit(Double_t x, Double_t y, Double_t z, Double_t charge);
    ~STHit();

    //!< Track ID setter
    void SetTrackID(Int_t trackID);
    //!< Hit setter
    void SetHit(TVector3 vec, Double_t charge);
    void SetHit(Double_t x, Double_t y, Double_t z, Double_t charge);
    //!< Position setter
    void SetPosition(TVector3 vec);
    void SetPosition(Double_t x, Double_t y, Double_t z);
    //!< Charge setter
    void SetCharge(Double_t charge);

    //!< Track ID getter
    Int_t GetTrackID();
    //!< Position getter
    TVector3 GetPosition();
    //!< Charge getter
    Double_t GetCharge();

  private:
    //!< Track ID
    Int_t fTrackID;
    //!< Position
    TVector3 fPosition;
    //!< Charge
    Double_t fCharge;

  ClassDef(STHit, 1);
};

#endif
