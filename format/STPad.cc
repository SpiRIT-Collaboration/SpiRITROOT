// =================================================
//  STPad Class
//
//  Description:
//    Container for a pad data
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 16
// ================================================= 

#include <iostream>

#include "STPad.hh"

ClassImp(STPad);

STPad::STPad()
{
  Initialize();
}

STPad::STPad(Int_t row, Int_t layer)
{
  Initialize();

  fRow = row;
  fLayer = layer;
}

STPad::~STPad()
{
}

void STPad::Initialize()
{
 fIsPedestalSubtracted = 0;

 memset(fRawAdc, 0, sizeof(fRawAdc));
 memset(fAdc, 0, sizeof(fAdc));

 fRow = -1;
 fLayer = -1;
}

// setters
void STPad::SetLayer(Int_t val)               { fLayer = val; }
void STPad::SetRow(Int_t val)                 { fRow = val; }
void STPad::SetRawADC(Int_t *val)             { memcpy(fRawAdc, val, sizeof(fRawAdc)); }
void STPad::SetRawADC(Int_t idx, Int_t val)   { fRawAdc[idx] = val; }
void STPad::SetPedestalSubtracted(Bool_t val) { fIsPedestalSubtracted = val; }
void STPad::SetGainCalibrated(Bool_t val)     { fIsGainCalibrated = val; }
void STPad::SetADC(Double_t *val)             { memcpy(fAdc, val, sizeof(fAdc)); }
void STPad::SetADC(Int_t idx, Double_t val)   { fAdc[idx] = val; }

STPad &STPad::operator= (STPad right)
{
  Initialize();

  fRow = right.GetRow();
  fLayer = right.GetLayer();

  memcpy(fRawAdc, right.GetRawADC(), sizeof(fRawAdc));
  memcpy(fAdc, right.GetADC(), sizeof(fAdc));

  fIsPedestalSubtracted = right.IsPedestalSubtracted();
  fIsGainCalibrated = right.IsGainCalibrated();

  return *this;
}

// getters
 Int_t  STPad::GetLayer()             { return fLayer; }
 Int_t  STPad::GetRow()               { return fRow; }
 Int_t *STPad::GetRawADC()            { return fRawAdc; }
 Int_t  STPad::GetRawADC(Int_t idx)   { return fRawAdc[idx]; }
Bool_t  STPad::IsPedestalSubtracted() { return fIsPedestalSubtracted; }
Bool_t  STPad::IsGainCalibrated()     { return fIsGainCalibrated; }

Double_t *STPad::GetADC()
{
  if (!fIsPedestalSubtracted) {
    std::cout << "== Pedestal subtraction is not done!" << std::endl;

    return 0;
  }

  return fAdc;
}

Double_t STPad::GetADC(Int_t idx)
{
  if (!fIsPedestalSubtracted) {
    std::cout << "== Pedestal subtraction is not done!" << std::endl;

    return -4;
  }

  return fAdc[idx];
}
