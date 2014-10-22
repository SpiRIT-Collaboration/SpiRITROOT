// =================================================
//  STCore Class
// 
//  Description:
//    Process CoBoFrame data into STRawEvent data
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 09. 25
// =================================================

#include <iostream>
#include <fstream>
#include <cmath>

#include "STCore.hh"

#include "STMap.hh"
#include "STPedestal.hh"
#include "STRawEvent.hh"

#include "GETDecoder.hh"
#include "GETFrame.hh"

ClassImp(STCore);

STCore::STCore()
{
  Initialize();
}

STCore::STCore(TString filename)
{
  Initialize();
  AddData(filename);
}

STCore::STCore(TString filename, Int_t numTbs)
{
  Initialize();
  AddData(filename);
  SetNumTbs(numTbs);
}

STCore::~STCore()
{
  delete fDecoderPtr;
  
  delete fPedestalPtr;
  delete fMapPtr;
}

void STCore::Initialize()
{
  fRawEventPtr = NULL;

  fMapPtr = new STMap();
  fPedestalPtr = new STPedestal();

  fDecoderPtr = new GETDecoder();
//  fDecoderPtr -> SetDebugMode(1);

  fIsData = kFALSE;
  fIsPedestalGenerationMode = kFALSE;
  fIsPedestalData = kFALSE;
  fIsInternalPedestal = kFALSE;
  fPedestalMode = kNoPedestal;
  fPedestalRMSFactor = 0;
  fIsFPNPedestal = kFALSE;

  fGainCalibrationPtr = new STGainCalibration();
  fIsGainCalibrationData = kFALSE;

  fSignalDelayPtr = new STSignalDelay();
  fIsSignalDelayData = kFALSE;

  fStartTb = 3;
  fAverageTbs = 20;

  fPrevEventNo = -1;
  fCurrEventNo = -1;
  fCurrFrameNo = 0;
}

Bool_t STCore::AddData(TString filename)
{
  return fDecoderPtr -> AddData(filename);
}

void STCore::SetNoAutoReload(Bool_t value)
{
  fDecoderPtr -> SetNoAutoReload(value);
}

void STCore::SetPedestalGenerationMode(Bool_t value)
{
  fIsPedestalGenerationMode = value;
}

void STCore::SetPositivePolarity(Bool_t value)
{
  fDecoderPtr -> SetPositivePolarity(value);
}

Bool_t STCore::SetData(Int_t value)
{
  fIsData = fDecoderPtr -> SetData(value);

  return fIsData;
}

Int_t STCore::GetNumData()
{
  return fDecoderPtr -> GetNumData();
}

TString STCore::GetDataName(Int_t index)
{
  return fDecoderPtr -> GetDataName(index);
}

void STCore::SetNumTbs(Int_t value)
{
  fNumTbs = value;
  fDecoderPtr -> SetNumTbs(value);
}

void STCore::SetInternalPedestal(Int_t startTb, Int_t averageTbs)
{
  if (fIsPedestalData) {
    fPedestalMode = kPedestalBothIE;
    std::cout << "== [STCore] Using both pedestal data is set!" << std::endl;
  } else {
    fPedestalMode = kPedestalInternal;
    std::cout << "== [STCore] Internal pedestal calculation will be done!" << std::endl;
  }

  fIsInternalPedestal = kTRUE;
  fStartTb = startTb;
  fAverageTbs = averageTbs;
}

Bool_t STCore::SetPedestalData(TString filename, Double_t rmsFactor)
{
  fIsPedestalData = fPedestalPtr -> SetPedestalData(filename);

  if (fIsPedestalData) {
    if (fIsInternalPedestal) {
      fPedestalMode = kPedestalBothIE;
      std::cout << "== [STCore] Using both pedestal data is set!" << std::endl;
    } else {
      fPedestalMode = kPedestalExternal;
      std::cout << "== [STCore] External pedestal data is set!" << std::endl;
    }

    fPedestalRMSFactor = rmsFactor;
  } else
    std::cout << "== [STCore] Pedestal data is not set! Check it exists!" << std::endl;

  return fIsPedestalData;
}

void STCore::SetPedestalFPN()
{
  fIsFPNPedestal = kTRUE;
  fPedestalMode = kPedestalFPN;

  std::cout << "== [STCore] Using FPN pedestal is set!" << std::endl;
}

Bool_t STCore::SetGainCalibrationData(TString filename)
{
  fIsGainCalibrationData = fGainCalibrationPtr -> SetGainCalibrationData(filename);

  return fIsGainCalibrationData;
}

void STCore::SetGainBase(Double_t constant, Double_t slope)
{
  if (!fIsGainCalibrationData) {
    std::cout << "== [STCore] Set gain calibration data first!" << std::endl;

    return;
  }

  fGainCalibrationPtr -> SetGainBase(constant, slope);
}

Bool_t STCore::SetSignalDelayData(TString filename)
{
  fIsSignalDelayData = fSignalDelayPtr -> SetSignalDelayData(filename);

  return fIsSignalDelayData;
}


Bool_t STCore::SetUAMap(TString filename)
{
  return fMapPtr -> SetUAMap(filename);
}

Bool_t STCore::SetAGETMap(TString filename)
{
  return fMapPtr -> SetAGETMap(filename);
}

STRawEvent *STCore::GetRawEvent(Int_t eventID)
{
  if (!fIsData) {
    std::cout << "== [STCore] Data file is not set!" << std::endl;

    return NULL;
  }

  if (fPedestalMode == kNoPedestal && !fIsPedestalGenerationMode)
    std::cout << "== [STCore] Pedestal data file is not set!" << std::endl;

  fPrevEventNo = eventID;

  if (fRawEventPtr != NULL)
    delete fRawEventPtr;

  fRawEventPtr = new STRawEvent();

  GETFrame *frame = NULL;
  while ((frame = fDecoderPtr -> GetFrame(fCurrFrameNo))) {
    if (fPrevEventNo == -1)
      fPrevEventNo = frame -> GetEventID();

    fCurrEventNo = frame -> GetEventID();

    if (fCurrEventNo == fPrevEventNo + 1) {
      fPrevEventNo = fCurrEventNo;
      return fRawEventPtr;
    } else if (fCurrEventNo > fPrevEventNo + 1) {
      fCurrFrameNo = 0;
      continue;
    } else if (fCurrEventNo < fPrevEventNo) {
      fCurrFrameNo++;
      continue;
    }

    fRawEventPtr -> SetEventID(fCurrEventNo);

    Int_t coboID = frame -> GetCoboID();
    Int_t frameType = fDecoderPtr -> GetFrameType();
    if (frameType == GETDecoder::kMergedID || frameType == GETDecoder::kMergedTime)
      coboID = fDecoderPtr -> GetCurrentInnerFrameID();
    Int_t asadID = frame -> GetAsadID();

    for (Int_t iAget = 0; iAget < 4; iAget++) {
      for (Int_t iCh = 0; iCh < 68; iCh++) {
        Int_t row, layer;
        fMapPtr -> GetRowNLayer(coboID, asadID, iAget, iCh, row, layer);

        if (row == -2 || layer == -2)
          continue;

        STPad *pad = new STPad(row, layer);
        Int_t *rawadc = frame -> GetRawADC(iAget, iCh);
        for (Int_t iTb = 0; iTb < fDecoderPtr -> GetNumTbs(); iTb++)
          pad -> SetRawADC(iTb, rawadc[iTb]);

        Int_t signalDelay = 0;
        if (fIsSignalDelayData) {
          Int_t uaIdx = fMapPtr -> GetUAIdx(coboID, asadID);
          signalDelay = ceil(fSignalDelayPtr -> GetSignalDelay(uaIdx));
        }

        if (fPedestalMode == kPedestalInternal) {
          frame -> CalcPedestal(iAget, iCh, fStartTb, fAverageTbs);
          frame -> SubtractPedestal(iAget, iCh);

          Double_t *adc = frame -> GetADC(iAget, iCh);
          for (Int_t iTb = 0; iTb < fDecoderPtr -> GetNumTbs(); iTb++) {
            Int_t delayedTb = iTb - signalDelay;
            Double_t delayedADC = 0;
            if (delayedTb < 0 || delayedTb >= fDecoderPtr -> GetNumTbs())
              delayedADC = 0;
            else
              delayedADC = adc[delayedTb];

            pad -> SetADC(iTb, delayedADC);
          }

          Int_t maxADCIdx = frame -> GetMaxADCIdx(iAget, iCh) + signalDelay;
          if (maxADCIdx < 0 || maxADCIdx >= fDecoderPtr -> GetNumTbs())
            maxADCIdx = 0;

          pad -> SetMaxADCIdx(maxADCIdx);
          pad -> SetPedestalSubtracted(kTRUE);
        } else if (fPedestalMode != kNoPedestal) {
          if (fPedestalMode == kPedestalBothIE)
            frame -> CalcPedestal(iAget, iCh, fStartTb, fAverageTbs);

          Double_t pedestal[512];
          Double_t pedestalSigma[512];

          if (fPedestalMode == kPedestalExternal || fPedestalMode == kPedestalBothIE) {
            fPedestalPtr -> GetPedestal(row, layer, pedestal, pedestalSigma);
            frame -> SetPedestal(iAget, iCh, pedestal, pedestalSigma);
          } else if (fPedestalMode == kPedestalFPN)
            frame -> SetFPNPedestal();

          frame -> SubtractPedestal(iAget, iCh, fPedestalRMSFactor);

          Double_t *adc = frame -> GetADC(iAget, iCh);

          if (fIsGainCalibrationData)
            fGainCalibrationPtr -> CalibrateADC(row, layer, fNumTbs, adc);

          for (Int_t iTb = 0; iTb < fDecoderPtr -> GetNumTbs(); iTb++) {
            Int_t delayedTb = iTb - signalDelay;
            Double_t delayedADC = 0;
            if (delayedTb < 0 || delayedTb >= fDecoderPtr -> GetNumTbs())
              delayedADC = 0;
            else
              delayedADC = adc[delayedTb];

            pad -> SetADC(iTb, delayedADC);
          }

          Int_t maxADCIdx = frame -> GetMaxADCIdx(iAget, iCh) + signalDelay;
          if (maxADCIdx < 0 || maxADCIdx >= fDecoderPtr -> GetNumTbs())
            maxADCIdx = 0;

          pad -> SetMaxADCIdx(maxADCIdx);
          pad -> SetPedestalSubtracted(kTRUE);
        }

        fRawEventPtr -> SetPad(pad);
        delete pad;
      }
    }

    if (frameType == GETDecoder::kMergedID || frameType == GETDecoder::kMergedTime) {
      Int_t currentInnerFrameID = fDecoderPtr -> GetCurrentInnerFrameID();
      Int_t numInnerFrames = fDecoderPtr -> GetNumMergedFrames();

      if (currentInnerFrameID + 1 == numInnerFrames) {
        fCurrFrameNo++;
        fPrevEventNo = fCurrEventNo;
        return fRawEventPtr;
      }
    } else
      fCurrFrameNo++;
  }

  return NULL;
}

Int_t STCore::GetNumTbs()
{
  return fDecoderPtr -> GetNumTbs();
}
