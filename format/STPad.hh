// =================================================
//  STPad Class
//
//  Description:
//    Container for a pad data
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 16
// ================================================= 

#ifndef STPAD_H
#define STPAD_H

#include "TROOT.h"
#include "TObject.h"

#include "STHit.hh"

class STPad : public TObject
{
  public:
    STPad();
    STPad(Int_t row, Int_t layer);
    ~STPad();

    void Clear();

    // setters
    void SetLayer(Int_t val);
    void SetRow(Int_t val);
    void SetSaturatedTb(Int_t val);
    void SetSaturatedTbMC(Int_t val);
  
    void SetRawADC(Int_t *val);
    void SetRawADC(Int_t idx, Int_t val);

    void SetIsSaturated(Bool_t val){fIsSaturated = val;};    
    void SetPedestalSubtracted(Bool_t val = kTRUE);
    void SetGainCalibrated(Bool_t val = kTRUE);
    void SetADC(Double_t *val);
    void SetADC(Int_t idx, Double_t val);

    STPad &operator= (STPad right);

    // getters
    Int_t GetLayer();
    Int_t GetRow();
    Double_t GetSaturatedTb();
    Double_t GetSaturatedTbMC();
  
    Bool_t IsSaturated(){return fIsSaturated;};
    Bool_t IsPedestalSubtracted();
    Bool_t IsGainCalibrated();

    Int_t *GetRawADC();
    Int_t GetRawADC(Int_t idx);

    Double_t *GetADC();
    Double_t GetADC(Int_t idx);

  private:
    Int_t fLayer;
    Int_t fRow;  
    Int_t fTb_sat    = 9999; //tB of saturation for time of saturation
    Int_t fTb_sat_MC = 9999; //tB of saturation for Monte carlo
  
    Int_t fRawAdc[512];
    Int_t fMaxAdcIdx;

    Bool_t fIsSaturated = false;
    Bool_t fIsPedestalSubtracted;
    Bool_t fIsGainCalibrated;
    Double_t fAdc[512];

  ClassDef(STPad, 4);
};

#endif
