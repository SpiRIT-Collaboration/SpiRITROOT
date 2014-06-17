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

  fPedestalSubtracted = 0;

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
        fAdc[index + iTb] = (adc < 0 || fRawAdc[index + iTb] == 0 ? 0 : adc);

        // Discard the first and the last bins
        if (iTb > 0 && iTb < 512 - 1) {
          if (fAdc[index + iTb] > fAdc[index + fMaxAdcIdx[index/512]])
            fMaxAdcIdx[index/512] = iTb;
        }
      }
    }
  }
  delete math;

  fPedestalSubtracted = 1;
}

Int_t GETFrame::GetMaxADCIdx(Int_t agetIdx, Int_t chIdx)
{
  //! \note This method is enabled after CalcPedestal() method.

  if (!fPedestalSubtracted) {
    std::cout << "== Run CalcPedstal(Int_t start, Int_t numBins) first!" << std::endl;

    return -1;
  }

  Int_t index = GetIndex(agetIdx, chIdx, 0)/512;

  return fMaxAdcIdx[index];
}

Double_t *GETFrame::GetADC(Int_t agetIdx, Int_t chIdx)
{
  //! \note This method is enabled after CalcPedestal() method.

  if (!fPedestalSubtracted) {
    std::cout << "== Run CalcPedstal(Int_t startTb, Int_t numTbs) first!" << std::endl;

    return NULL;
  }

  Int_t index = GetIndex(agetIdx, chIdx, 0);

  return fAdc + index;
}

Double_t GETFrame::GetADC(Int_t agetIdx, Int_t chIdx, Int_t buckIdx)
{
  //! \note This method is enabled after CalcPedestal() method.

  if (!fPedestalSubtracted) {
    std::cout << "== Run CalcPedstal(Int_t start, Int_t numBins) first!" << std::endl;

    return -1;
  }

  Int_t index = GetIndex(agetIdx, chIdx, buckIdx);

  return fAdc[index]; 
}

Double_t GETFrame::GetPedestal(Int_t agetIdx, Int_t chIdx)
{
  if (!fPedestalSubtracted) {
    std::cout << "== Run CalcPedstal(Int_t start, Int_t numBins) first!" << std::endl;

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
