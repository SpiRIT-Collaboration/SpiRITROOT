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
  SetNumTbs(512);
  SetWindow(512, 0);
}

STCore::STCore(TString filename, Int_t numTbs, Int_t windowNumTbs, Int_t windowStartTb)
{
  Initialize();
  AddData(filename);
  SetNumTbs(numTbs);
  SetWindow(windowNumTbs, windowStartTb);
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
  fPlotPtr = NULL;

  fDecoderPtr = new GETDecoder();
//  fDecoderPtr -> SetDebugMode(1);

  fIsData = kFALSE;
  fIsPedestalGenerationMode = kFALSE;
  fIsPedestalData = kFALSE;
  fIsInternalPedestal = kFALSE;
  fPedestalMode = kNoPedestal;
  fPedestalRMSFactor = 0;
  fIsFPNPedestal = kFALSE;
  fFPNSigmaThreshold = 5;

  fGainCalibrationPtr = new STGainCalibration();
  fIsGainCalibrationData = kFALSE;

  fNumTbs = 512;

  fWindowNumTbs = 512;
  fWindowStartTb = 0;

  fPedestalStartTb = 3;
  fAverageTbs = 20;

  fPrevEventNo = -1;
  fCurrEventNo = -1;
  fCurrFrameNo = 0;

  fOldData = kFALSE;
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

  fPrevEventNo = -1;
  fCurrEventNo = -1;
  fCurrFrameNo = 0;

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

void STCore::SetWindow(Int_t numTbs, Int_t startTb)
{
  fWindowNumTbs = numTbs;
  fWindowStartTb = startTb;
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
  fPedestalStartTb = startTb;
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

void STCore::SetFPNPedestal(Double_t sigmaThreshold)
{
  fIsFPNPedestal = kTRUE;
  fPedestalMode = kPedestalFPN;
  fFPNSigmaThreshold = sigmaThreshold;

  std::cout << "== [STCore] Using FPN pedestal is set!" << std::endl;
}

Bool_t STCore::SetGainCalibrationData(TString filename, TString dataType)
{
  fIsGainCalibrationData = fGainCalibrationPtr -> SetGainCalibrationData(filename, dataType);

  std::cout << "== [STCore] Gain calibration data is set!" << std::endl;
  return fIsGainCalibrationData;
}

void STCore::SetGainReference(Int_t row, Int_t layer)
{
  if (!fIsGainCalibrationData) {
    std::cout << "== [STCore] Set gain calibration data first!" << std::endl;

    return;
  }

  fGainCalibrationPtr -> SetGainReference(row, layer);
}

void STCore::SetGainReference(Double_t constant, Double_t linear, Double_t quadratic)
{
  if (!fIsGainCalibrationData) {
    std::cout << "== [STCore] Set gain calibration data first!" << std::endl;

    return;
  }

  fGainCalibrationPtr -> SetGainReference(constant, linear, quadratic);
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

    Int_t frameType = fDecoderPtr -> GetFrameType();

    if ((frameType == GETDecoder::kMergedID || frameType == GETDecoder::kMergedTime) && !(fRawEventPtr -> IsGood())) {
      Int_t currentInnerFrameID = fDecoderPtr -> GetCurrentInnerFrameID();
      Int_t numInnerFrames = fDecoderPtr -> GetNumMergedFrames();

      while (!(currentInnerFrameID + 1 == numInnerFrames)) {
        fDecoderPtr -> GetFrame(fCurrFrameNo);

        currentInnerFrameID = fDecoderPtr -> GetCurrentInnerFrameID();
        numInnerFrames = fDecoderPtr -> GetNumMergedFrames();
      }

      fCurrFrameNo++;
      fPrevEventNo = fCurrEventNo;
      return fRawEventPtr;
    }

    fRawEventPtr -> SetEventID(fCurrEventNo);

    Int_t coboID = frame -> GetCoboID();

    if (fOldData == kTRUE && (frameType == GETDecoder::kMergedID || frameType == GETDecoder::kMergedTime))
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
        for (Int_t iTb = 0; iTb < fWindowNumTbs; iTb++)
          pad -> SetRawADC(iTb, rawadc[fWindowStartTb + iTb]);

        if (fPedestalMode == kPedestalInternal) {
          frame -> CalcPedestal(iAget, iCh, fPedestalStartTb, fAverageTbs);
          frame -> SubtractPedestal(iAget, iCh);

          Double_t *adc = frame -> GetADC(iAget, iCh);
          for (Int_t iTb = 0; iTb < fWindowNumTbs; iTb++)
            pad -> SetADC(iTb, adc[fWindowStartTb + iTb]);

          Int_t maxADCIdx = frame -> GetMaxADCIdx(iAget, iCh);

          if (maxADCIdx < fWindowStartTb || maxADCIdx > fWindowStartTb + fWindowNumTbs) {
            Double_t maxADC = -9999;
            for (Int_t iTb = 0; iTb < fWindowNumTbs; iTb++) {
              if (maxADC < adc[iTb + fWindowStartTb]) {
                maxADC = adc[iTb + fWindowStartTb];
                maxADCIdx = iTb;
              }
            }
          } else
            maxADCIdx -= fWindowStartTb;

          pad -> SetMaxADCIdx(maxADCIdx);
          pad -> SetPedestalSubtracted(kTRUE);
        } else if (fPedestalMode != kNoPedestal) {
          if (fPedestalMode == kPedestalBothIE)
            frame -> CalcPedestal(iAget, iCh, fPedestalStartTb, fAverageTbs);

          Double_t pedestal[512];
          Double_t pedestalSigma[512];

          if (fPedestalMode == kPedestalExternal || fPedestalMode == kPedestalBothIE) {
            fPedestalPtr -> GetPedestal(row, layer, pedestal, pedestalSigma);
            frame -> SetPedestal(iAget, iCh, pedestal, pedestalSigma);
          } else if (fPedestalMode == kPedestalFPN)
            frame -> SetFPNPedestal(fFPNSigmaThreshold);

          Bool_t good = frame -> SubtractPedestal(iAget, iCh, fPedestalRMSFactor);
          fRawEventPtr -> SetIsGood(good);
          if (!good) {
            delete pad;

            iAget = 4;
            iCh = 68;

            continue;
          }

          Double_t *adc = frame -> GetADC(iAget, iCh);

          if (fIsGainCalibrationData)
            fGainCalibrationPtr -> CalibrateADC(row, layer, fNumTbs, adc);

          for (Int_t iTb = 0; iTb < fWindowNumTbs; iTb++)
            pad -> SetADC(iTb, adc[fWindowStartTb + iTb]);

          Int_t maxADCIdx = frame -> GetMaxADCIdx(iAget, iCh);

          if (maxADCIdx < fWindowStartTb || maxADCIdx > fWindowStartTb + fWindowNumTbs) {
            Double_t maxADC = -9999;
            for (Int_t iTb = 0; iTb < fWindowNumTbs; iTb++) {
              if (maxADC < adc[iTb + fWindowStartTb]) {
                maxADC = adc[iTb + fWindowStartTb];
                maxADCIdx = iTb;
              }
            }
          } else
            maxADCIdx -= fWindowStartTb;

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

void STCore::SetOldData(Bool_t oldData)
{
  fOldData = oldData;
}

STPlot *STCore::GetSTPlot()
{
  if (fPlotPtr == NULL)
    fPlotPtr = new STPlot(this);

  return fPlotPtr;
}
