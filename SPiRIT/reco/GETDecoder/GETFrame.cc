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

ClassImp(GETFrame);

GETFrame::GETFrame()
{
  fNumTbs = 512;

  fEventIdx = 0;
  fCoboIdx = 0;
  fAsadIdx = 0;
  fPolarity = -1;

  memset(fRawAdc, 0, sizeof(Int_t)*4*68*512);

  memset(fMaxAdcIdx, 0, sizeof(Int_t)*4*68);
  memset(fAdc, 0, sizeof(Double_t)*4*68*512);

  memset(fIsPedestalSubtracted, kFALSE, sizeof(Bool_t)*4*68);

  memset(fIsCalcPedestalUsed, kFALSE, sizeof(Bool_t)*4*68);
  memset(fInternalPedestal, 0, sizeof(Double_t)*4*68);

  memset(fIsSetPedestalUsed, kFALSE, sizeof(Bool_t)*4*68);
  memset(fPedestalDataMean, 0, sizeof(Double_t)*4*68);
  memset(fPedestalData, 0, sizeof(Double_t)*4*68*512);
  memset(fPedestalSigmaData, 0, sizeof(Double_t)*4*68*512);

  fIsFPNPedestalUsed = kFALSE;
  fFPNSigmaThreshold = 0;
  memset(fFPNPedestalMean, 0, sizeof(Double_t)*4*4);
  fFPNStartTb = 3;
  fFPNAverageTbs = 10;

  fMath = new GETMath();
}

GETFrame::~GETFrame() {}

void GETFrame::SetNumTbs(Int_t value)    { fNumTbs = value; }
void GETFrame::SetEventID(UInt_t value)  { fEventIdx = value; }
void GETFrame::SetCoboID(UShort_t value) { fCoboIdx = value; }
void GETFrame::SetAsadID(UShort_t value) { fAsadIdx = value; } 
void GETFrame::SetFrameID(Int_t value)   { fFrameIdx = value; } 
void GETFrame::SetPolarity(Int_t value)  { fPolarity = value; } 
void GETFrame::SetRawADC(UShort_t agetIdx, UShort_t chIdx, UShort_t buckIdx, UShort_t value)
{
  Int_t index = GetIndex(agetIdx, chIdx, buckIdx);

  fRawAdc[index] = value;
}

 Int_t  GETFrame::GetNumTbs()   { return fNumTbs; }
UInt_t  GETFrame::GetEventID()  { return fEventIdx; } 
 Int_t  GETFrame::GetCoboID()   { return fCoboIdx; } 
 Int_t  GETFrame::GetAsadID()   { return fAsadIdx; } 
 Int_t  GETFrame::GetFrameID()  { return fFrameIdx; } 
 Int_t  GETFrame::GetPolarity() { return fPolarity; } 
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

void GETFrame::CalcPedestal(Int_t agetIdx, Int_t chIdx, Int_t startTb, Int_t numTbs)
{
  /*!
    * Calculates pedestal and its RMS value using numTbs time buckets starting
    * from startTb and subtract the pedestal value from raw ADC.
    * After this method, user can get the pedestal-subtracted values using
    * GetADC() method.
  **/
  
  Int_t index = GetIndex(agetIdx, chIdx, 0);

  fMath -> Reset();
  for (Int_t iTb = startTb; iTb < startTb + numTbs; iTb++)
    fMath -> Add(fRawAdc[index + iTb]);

  fInternalPedestal[index/512] = fMath -> GetMean();

  fIsCalcPedestalUsed[index/512] = kTRUE;
}

void GETFrame::SetPedestal(Int_t agetIdx, Int_t chIdx, Double_t *pedestal, Double_t *pedestalSigma) {
  /*!
    * Using the calculated pedestal input, subtract it from the signal.
    * This method has different way to treat pedestal from using internal pedestal calculation.
    * Internal case only gives one pedestal value and sigma value for a channel.
    * This method calculates pedestal and sigma value for each time bin.
    * Therefore, after using this method GetPedestal() method is disabled.
  **/
  Int_t index = GetIndex(agetIdx, chIdx, 0);

  memcpy(pedestal, fPedestalData + index, sizeof(Double_t)*fNumTbs);
  memcpy(pedestalSigma, fPedestalSigmaData + index, sizeof(Double_t)*fNumTbs);

  fMath -> Reset();
  for (Int_t iTb = 1; iTb < fNumTbs - 1; iTb++)
    fMath -> Add(pedestal[iTb]);

  fPedestalDataMean[index/512] = fMath -> GetMean();

  fIsSetPedestalUsed[index/512] = kTRUE;
}

void GETFrame::SetFPNPedestal(Int_t sigmaThreshold) {
  /**
    * Use FPN channels as pedestal.
   **/

  fFPNSigmaThreshold = sigmaThreshold;

  for (Int_t iAget = 0; iAget < 4; iAget++) {
    for (Int_t iFPN = 0; iFPN < 4; iFPN++) {
      Int_t fpnCh = (iFPN + 1 + iFPN/2)*11 + iFPN/2;
      Int_t fpnIndex = GetIndex(iAget, fpnCh, 0);

      fMath -> Reset();
      for (Int_t iTb = fFPNStartTb; iTb < fFPNStartTb + fFPNAverageTbs; iTb++)
        fMath -> Add(fRawAdc[fpnIndex + iTb]);

      fFPNPedestalMean[iAget*4 + iFPN] = fMath -> GetMean();;
    }
  }

  fIsFPNPedestalUsed = kTRUE;
}

Bool_t GETFrame::SubtractPedestal(Int_t agetIdx, Int_t chIdx, Double_t rmsFactor)
{
  Int_t index = GetIndex(agetIdx, chIdx, 0);

  if (!fIsCalcPedestalUsed[index/512] && !fIsSetPedestalUsed[index/512] && !fIsFPNPedestalUsed) {
    std::cout << "== [GETFrame] Run CalcPedestal(), SetPedestal(), or SetFPNPedestal() first!" << std::endl;
    
    return kFALSE;
  }

  if (!fIsFPNPedestalUsed) {
    for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
      Double_t pedestal = GetPedestal(agetIdx, chIdx, iTb, rmsFactor);
      Double_t adc = 0;
      if (fPolarity < 0)
        adc = pedestal - fRawAdc[index + iTb];
      else
        adc = fRawAdc[index + iTb] - pedestal;

      fAdc[index + iTb] = ((adc < 0 || fRawAdc[index + iTb] == 0) ? 0 : adc);
    }
  } else {
    Int_t fpnCh = GetFPNChannel(agetIdx, chIdx);
    Int_t fpnArrayIndex = GetFPNArrayIndex(agetIdx, chIdx);

    Int_t fpnIndex = GetIndex(agetIdx, fpnCh, 0);

    Int_t startTb = fFPNStartTb;

    while (1) {
      fMath -> Reset();
      for (Int_t iTb = startTb; iTb < startTb + fFPNAverageTbs; iTb++)
        fMath -> Add(fRawAdc[index + iTb]);

      if (fMath -> GetRMS() < fFPNSigmaThreshold)
        break;

      startTb += fFPNAverageTbs;

      if (startTb > fNumTbs - fFPNAverageTbs - 3) {
        std::cout << "There's no part satisfying sigma threshold " << fFPNSigmaThreshold << "!" << std::endl;
        return kFALSE;
      }
    }

    Double_t baselineDiff = 0;
    if (startTb == fFPNStartTb)
      baselineDiff = fFPNPedestalMean[fpnArrayIndex] - fMath -> GetMean();
    else { 
      baselineDiff = - fMath -> GetMean();

      fMath -> Reset();
      for (Int_t iTb = startTb; iTb < startTb + fFPNAverageTbs; iTb++)
        fMath -> Add(fRawAdc[fpnIndex + iTb]);

      baselineDiff += fMath -> GetMean();
    }

    for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
      Double_t adc = 0;
      if (fPolarity < 0)
        adc = (fRawAdc[fpnIndex + iTb] - baselineDiff) - fRawAdc[index + iTb];
      else
        adc = fRawAdc[index + iTb] - (fRawAdc[fpnIndex + iTb] - baselineDiff);

      fAdc[index + iTb] = ((adc < 0 || fRawAdc[index + iTb] == 0) ? 0 : adc);
    }
  }

  FindMaxIdx(agetIdx, chIdx);

  fIsPedestalSubtracted[index/512] = kTRUE;

  return kTRUE;
}

void GETFrame::FindMaxIdx(Int_t agetIdx, Int_t chIdx) {
  Int_t index = GetIndex(agetIdx, chIdx, 0);

  // Discard the first and the last bins
  for (Int_t iTb = 3; iTb < fNumTbs - 3; iTb++) {
    if (fAdc[index + iTb] > fAdc[index + fMaxAdcIdx[index/512]])
      fMaxAdcIdx[index/512] = iTb;
  }
}

Int_t GETFrame::GetMaxADCIdx(Int_t agetIdx, Int_t chIdx)
{
  //! \note This method is enabled after CalcPedestal() method.

  Int_t index = GetIndex(agetIdx, chIdx, 0)/512;

  if (!fIsPedestalSubtracted[index]) {
    std::cout << "== [GETFrame] Run SubtractPedestal() first!" << std::endl;

    return -1;
  }

  return fMaxAdcIdx[index];
}

Double_t *GETFrame::GetADC(Int_t agetIdx, Int_t chIdx)
{
  //! \note This method is enabled after SubtractPedestal() method.

  Int_t index = GetIndex(agetIdx, chIdx, 0);

  if (!fIsPedestalSubtracted[index/512]) {
    std::cout << "== [GETFrame] Run SubtractPedestal() first!" << std::endl;

    return NULL;
  }

  return fAdc + index;
}

Double_t GETFrame::GetADC(Int_t agetIdx, Int_t chIdx, Int_t buckIdx)
{
  //! \note This method is enabled after SubtractPedestal() method.

  Int_t index = GetIndex(agetIdx, chIdx, buckIdx);

  if (!fIsPedestalSubtracted[index/512]) {
    std::cout << "== [GETFrame] Run SubtractPedestal() first!" << std::endl;

    return -1;
  }

  return fAdc[index]; 
}

Double_t GETFrame::GetPedestal(Int_t agetIdx, Int_t chIdx, Int_t buckIdx, Double_t rmsFactor)
{
  Int_t index = GetIndex(agetIdx, chIdx, 0);

  if (!fIsCalcPedestalUsed[index/512] && !fIsSetPedestalUsed[index/512]) {
    std::cout << "== [GETFrame] Run CalcPedstal() or SetPedestal() first!" << std::endl;

    return -1;
  }

  if (fIsCalcPedestalUsed[index/512] && fIsSetPedestalUsed[index/512]) {
    if (fPolarity < 0)
      return fPedestalData[index + buckIdx] - rmsFactor*fPedestalSigmaData[index + buckIdx] - (fPedestalDataMean[index/512] - fInternalPedestal[index/512]);
    else
      return fPedestalData[index + buckIdx] + rmsFactor*fPedestalSigmaData[index + buckIdx] - (fPedestalDataMean[index/512] - fInternalPedestal[index/512]);
  } else if (fIsCalcPedestalUsed[index/512]) {
    return fInternalPedestal[index/512];
  } else if (fIsSetPedestalUsed[index/512]) {
    if (fPolarity < 0)
      return fPedestalData[index + buckIdx] - rmsFactor*fPedestalSigmaData[index + buckIdx];
    else
      return fPedestalData[index + buckIdx] + rmsFactor*fPedestalSigmaData[index + buckIdx];
  }
}

Int_t GETFrame::GetIndex(Int_t agetIdx, Int_t chIdx, Int_t buckIdx)
{
  if (agetIdx > 3) {
    std::cout << "== [GETFrame] AGET number should be in [0,3]!" << std::endl;

    return -1;
  } else if (chIdx > 67) {
    std::cout << "== [GETFrame] Channel number should be in [0,67]!" << std::endl;

    return -1;
  } else if (buckIdx > fNumTbs - 1) {
    std::cout << "== [GETFrame] Channel number should be in [0," << fNumTbs - 1 << "]!" << std::endl;

    return -1;
  }

  return agetIdx*68*512 + chIdx*512 + buckIdx;
}

Int_t GETFrame::GetFPNChannel(Int_t agetIdx, Int_t chIdx)
{
  if (agetIdx > 3) {
    std::cout << "== [GETFrame] AGET number should be in [0,3]!" << std::endl;

    return -1;
  } else if (chIdx > 67) {
    std::cout << "== [GETFrame] Channel number should be in [0,67]!" << std::endl;

    return -1;
  }

  Int_t fpn = -1;

       if (chIdx < 17) fpn = 11;
  else if (chIdx < 34) fpn = 22;
  else if (chIdx < 51) fpn = 45;
  else                 fpn = 56;

  return fpn;
}

Int_t GETFrame::GetFPNArrayIndex(Int_t agetIdx, Int_t chIdx)
{
  if (agetIdx > 3) {
    std::cout << "== [GETFrame] AGET number should be in [0,3]!" << std::endl;

    return -1;
  } else if (chIdx > 67) {
    std::cout << "== [GETFrame] Channel number should be in [0,67]!" << std::endl;

    return -1;
  }

  Int_t fpnChannel = GetFPNChannel(agetIdx, chIdx);

  return agetIdx*4 + fpnChannel/11 - 1 - fpnChannel%11;
}
