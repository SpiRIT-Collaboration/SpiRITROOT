// =================================================
//  GETFrame Class
// 
//  Author:
//    Genie Jhang ( geniejhang@majimak.com )
//  
//  Log:
//    - 2013. 09. 23
//      Start writing class
// =================================================

#include <iostream>
#include <cmath>

#include "GETFrame.hh"
#include "GETMath.hh"

ClassImp(GETFrame);

GETFrame::GETFrame()
{
  fEventIdx = 0;
  fCoboIdx = 0;
  fAsadIdx = 0;

  fIsPedestalSubtracted = 0;
  fIsSetPedestalUsed = 0;

  for (Int_t i = 0; i < 4*68*512; i++) {
    fRawAdc[i] = 0;
    fMaxAdcIdx[i/512] = 0;
    fAdc[i] = 0;
    fPedestal[i/512] = 0;
  }
}

GETFrame::~GETFrame()
{}

void GETFrame::SetEventID(UInt_t value)
{
  fEventIdx = value;
}

void GETFrame::SetCoboID(UShort_t value)
{
  fCoboIdx = value;
}

void GETFrame::SetAsadID(UShort_t value)
{
  fAsadIdx = value;
}

void GETFrame::SetFrameID(Int_t value)
{
  fFrameIdx = value;
}

void GETFrame::SetRawADC(UShort_t agetIdx, UShort_t chIdx, UShort_t buckIdx, UShort_t value)
{
  Int_t index = GetIndex(agetIdx, chIdx, buckIdx);

  fRawAdc[index] = value;
}

UInt_t GETFrame::GetEventID()
{
  return fEventIdx;
}

Int_t GETFrame::GetCoboID()
{
  return fCoboIdx;
}

Int_t GETFrame::GetAsadID()
{
  return fAsadIdx;
}

Int_t GETFrame::GetFrameID()
{
  return fFrameIdx;
}

Int_t *GETFrame::GetRawADC(Int_t agetIdx, Int_t chIdx)
{
  Int_t index = GetIndex(agetIdx, chIdx, 0);

  return fRawAdc + index;
}

Int_t GETFrame::GetRawADC(Int_t agetIdx, Int_t chIdx, Int_t buckIdx)
{
  Int_t index = GetIndex(agetIdx, chIdx, buckIdx);

  return fRawAdc[index]; 
}

void GETFrame::CalcPedestal(Int_t startTb, Int_t numTbs)
{
  /*!
    * Calculates pedestal and its RMS value using numTbs time buckets starting
    * from startTb and subtract the pedestal value from raw ADC.
    * After this method, user can get the pedestal-subtracted values using
    * GetADC() method.
  **/
  
  GETMath *math = new GETMath();
  for (Int_t iAget = 0; iAget < 4; iAget++) {
    for (Int_t iCh = 0; iCh < 68; iCh++) {
      Int_t index = GetIndex(iAget, iCh, 0);

      math -> Reset();
      for (Int_t iTb = startTb; iTb < startTb + numTbs; iTb++)
        math -> Add(fRawAdc[index + iTb]);

      fPedestal[index/512] = math -> GetMean();

      for (Int_t iTb = 0; iTb < 512; iTb++) {
        Double_t adc = (math -> GetMean()) - fRawAdc[index + iTb];
        fAdc[index + iTb] = (adc < 0 || fRawAdc[index + iTb] == 0 ? 0 : adc); }

      FindMaxIdx(iAget, iCh);
    }
  }
  delete math;

  fIsPedestalSubtracted = 1;
}

void GETFrame::SetPedestal(Int_t agetIdx, Int_t chIdx, Double_t *pedestal, Double_t *pedestalSigma) {
  /*!
    * Using the calculated pedestal input, subtract it from the signal.
    * This method has different way to treat pedestal from using internal pedestal calculation.
    * Internal case only gives one pedestal value and sigma value for a channel.
    * This method calculates pedestal and sigma value for each time bin.
    * Therefore, after using this method GetPedestal() methoid is disabled.
  **/
  Int_t index = GetIndex(agetIdx, chIdx, 0);

  for (Int_t iTb = 0; iTb < 512; iTb++) {
    Double_t adc = pedestal[iTb] - 2*pedestalSigma[iTb] - fRawAdc[index + iTb];
    fAdc[index + iTb] = (adc < 0 || fRawAdc[index + iTb] == 0 ? 0 : adc);
  }

  FindMaxIdx(agetIdx, chIdx);

  fIsPedestalSubtracted = 1;
  fIsSetPedestalUsed = 1;
}

void GETFrame::FindMaxIdx(Int_t agetIdx, Int_t chIdx) {
  Int_t index = GetIndex(agetIdx, chIdx, 0);

  // Discard the first and the last bins
  for (Int_t iTb = 1; iTb < 511; iTb++) {
    if (fAdc[index + iTb] > fAdc[index + fMaxAdcIdx[index/512]])
      fMaxAdcIdx[index/512] = iTb;
  }
}

Int_t GETFrame::GetMaxADCIdx(Int_t agetIdx, Int_t chIdx)
{
  //! \note This method is enabled after CalcPedestal() method.

  if (!fIsPedestalSubtracted) {
    std::cout << "== Run CalcPedestal() or SetPedestal() first!" << std::endl;

    return -1;
  }

  Int_t index = GetIndex(agetIdx, chIdx, 0)/512;

  return fMaxAdcIdx[index];
}

Double_t *GETFrame::GetADC(Int_t agetIdx, Int_t chIdx)
{
  //! \note This method is enabled after CalcPedestal() method.

  if (!fIsPedestalSubtracted) {
    std::cout << "== Run CalcPedestal() or SetPedestal() first!" << std::endl;

    return NULL;
  }

  Int_t index = GetIndex(agetIdx, chIdx, 0);

  return fAdc + index;
}

Double_t GETFrame::GetADC(Int_t agetIdx, Int_t chIdx, Int_t buckIdx)
{
  //! \note This method is enabled after CalcPedestal() method.

  if (!fIsPedestalSubtracted) {
    std::cout << "== Run CalcPedestal() or SetPedestal() first!" << std::endl;

    return -1;
  }

  Int_t index = GetIndex(agetIdx, chIdx, buckIdx);

  return fAdc[index]; 
}

Double_t GETFrame::GetPedestal(Int_t agetIdx, Int_t chIdx)
{
  if (!fIsPedestalSubtracted) {
    std::cout << "== Run CalcPedstal()  first!" << std::endl;

    return -1;
  } else if (fIsSetPedestalUsed) {
    std::cout << "== GetPedestal() is disabled by SetPedestal()!" << std::endl;

    return -1;
  }

  Int_t index = GetIndex(agetIdx, chIdx, 0)/512;
  return fPedestal[index];
}

Int_t GETFrame::GetIndex(Int_t agetIdx, Int_t chIdx, Int_t buckIdx)
{
  if (agetIdx > 3) {
    std::cout << "== AGET number should be in [0,3]!" << std::endl;

    return -1;
  } else if (chIdx > 67) {
    std::cout << "== Channel number should be in [0,67]!" << std::endl;

    return -1;
  } else if (buckIdx > 512) {
    std::cout << "== Channel number should be in [0," << 512 << "]!" << std::endl;

    return -1;
  }

  return agetIdx*68*512 + chIdx*512 + buckIdx;
}
