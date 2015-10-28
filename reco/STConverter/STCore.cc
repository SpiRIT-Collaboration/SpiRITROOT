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
#include <thread>

#include "STCore.hh"

#include "STMap.hh"
#include "STPedestal.hh"
#include "STRawEvent.hh"

#include "GETDecoder.hh"
#include "GETFrame.hh"

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

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
}

STCore::STCore(TString filename, Int_t numTbs, Int_t windowNumTbs, Int_t windowStartTb)
{
  Initialize();
  AddData(filename);
  SetNumTbs(numTbs);
}

STCore::~STCore()
{
  delete fPedestalPtr;
  delete fMapPtr;
}

void STCore::Initialize()
{
  fRawEventPtr = new STRawEvent();

  fMapPtr = new STMap();
  fPedestalPtr = new STPedestal();
  fPlotPtr = NULL;

  fDecoderPtr[0] = new GETDecoder();
//  fDecoderPtr[0] -> SetDebugMode(1);
  fPadArray = new TClonesArray("STPad", 12096);

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

  fPedestalStartTb = 3;
  fAverageTbs = 20;

  fPrevEventNo = 0;
  fCurrEventNo = 0;
  fNextEventNo = 0;
  memset(fCurrFrameNo, 0, sizeof(Int_t)*12);
  memset(fNumCurrEventFrames, 0, sizeof(Int_t)*12);

  fOldData = kFALSE;
  fIsSeparatedData = kFALSE;
}

Bool_t STCore::AddData(TString filename, Int_t coboIdx)
{
  return fDecoderPtr[coboIdx] -> AddData(filename);
}

void STCore::SetNoAutoReload(Bool_t value)
{
  fDecoderPtr[0] -> SetNoAutoReload(value);

  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++)
      fDecoderPtr[iCobo] -> SetNoAutoReload(value);
}

void STCore::SetPedestalGenerationMode(Bool_t value)
{
  fIsPedestalGenerationMode = value;
}

void STCore::SetPositivePolarity(Bool_t value)
{
  fDecoderPtr[0] -> SetPositivePolarity(value);

  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++)
      fDecoderPtr[iCobo] -> SetPositivePolarity(value);
}

Bool_t STCore::SetData(Int_t value)
{
  fIsData = fDecoderPtr[0] -> SetData(value);
  Int_t frameType = (fDecoderPtr[0] -> GetFrameType() == GETDecoder::kNormal ? GETDecoder::kNormal : -1);

  if (fIsSeparatedData) {
    for (Int_t iCobo = 1; iCobo < 12; iCobo++) {
      fIsData &= fDecoderPtr[iCobo] -> SetData(value);
      frameType = (fDecoderPtr[iCobo] -> GetFrameType() == GETDecoder::kNormal ? GETDecoder::kNormal : -1);
    }

    if (frameType != GETDecoder::kNormal) {
      std::cout << cRED << "== [STCore] When using separated data, only accepted are normal(not merged) frame data files!" << cNORMAL << std::endl;

      fIsData = kFALSE;
    }
  }

  fPrevEventNo = 0;
  fCurrEventNo = 0;
  fNextEventNo = 0;
  memset(fCurrFrameNo, 0, sizeof(Int_t)*12);

  return fIsData;
}

Int_t STCore::GetNumData(Int_t coboIdx)
{
  return fDecoderPtr[coboIdx] -> GetNumData();
}

TString STCore::GetDataName(Int_t index, Int_t coboIdx)
{
  return fDecoderPtr[coboIdx] -> GetDataName(index);
}

void STCore::SetNumTbs(Int_t value)
{
  fNumTbs = value;
  fDecoderPtr[0] -> SetNumTbs(value);

  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++)
      fDecoderPtr[iCobo] -> SetNumTbs(value);
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

void STCore::ProcessCobo(Int_t coboIdx)
{
  Int_t iSkipper = 0;

  Bool_t skipped = kFALSE;
  fNumCurrEventFrames[coboIdx] = 0;

  while (1) {
    GETFrame *frame = fDecoderPtr[coboIdx] -> GetFrame(fCurrFrameNo[coboIdx] + iSkipper);

    if (frame == NULL)
      break;

    if (frame -> GetEventID() != fCurrEventNo) {
      iSkipper++;

      skipped = kTRUE;

      continue;
    }

    Int_t coboID = frame -> GetCoboID();
    Int_t asadID = frame -> GetAsadID();

//    std::cout << "coboIdx: " << coboIdx << " skipped: " << skipped << " fCurrFrameNo[coboIdx]: " << fCurrFrameNo[coboIdx];
//    std::cout << " iSkipper: " << iSkipper << " Taken: " << fCurrFrameNo[coboIdx] + iSkipper << " fNumCurrEventFrames[coboIdx]: " << fNumCurrEventFrames[coboIdx] << std::endl;

    fCurrEventFrameNo[coboIdx][asadID] = fCurrFrameNo[coboIdx] + iSkipper;
    fNumCurrEventFrames[coboIdx]++;

    if (!skipped)
      fCurrFrameNo[coboIdx]++;
    else
      iSkipper++;

    for (Int_t iAget = 0; iAget < 4; iAget++) {
      for (Int_t iCh = 0; iCh < 68; iCh++) {
        Int_t row, layer;
        fMapPtr -> GetRowNLayer(coboID, asadID, iAget, iCh, row, layer);

        if (row == -2 || layer == -2)
          continue;

        STPad *pad = new ((*fPadArray)[row*112 + layer]) STPad(row, layer);
        Int_t *rawadc = frame -> GetRawADC(iAget, iCh);
        for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
          pad -> SetRawADC(iTb, rawadc[iTb]);

        if (fPedestalMode == kPedestalInternal) {
          frame -> CalcPedestal(iAget, iCh, fPedestalStartTb, fAverageTbs);
          frame -> SubtractPedestal(iAget, iCh);

          Double_t *adc = frame -> GetADC(iAget, iCh);
          for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
            pad -> SetADC(iTb, adc[iTb]);

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
            iAget = 4;
            iCh = 68;

            continue;
          }

          Double_t *adc = frame -> GetADC(iAget, iCh);

          if (fIsGainCalibrationData)
            fGainCalibrationPtr -> CalibrateADC(row, layer, fNumTbs, adc);

          for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
            pad -> SetADC(iTb, adc[iTb]);

          pad -> SetPedestalSubtracted(kTRUE);
        }
      }
    }

    if (fNumCurrEventFrames[coboIdx] == 4) {
      fCurrFrameNo[coboIdx] = (fCurrEventFrameNo[coboIdx][0] > 3 ? fCurrEventFrameNo[coboIdx][0] - 3 : 0);

      break;
    }
  }

  if (fNumCurrEventFrames[coboIdx] < 4)
    std::cout << "== [STCore] Warning: There're less than 4 AsAds' frame in this event! (Event ID: " << fCurrEventNo << ")" << std::endl;
}

Bool_t STCore::SetWriteFile(TString filename, Int_t coboIdx, Bool_t overwrite)
{
  return fDecoderPtr[coboIdx] -> SetWriteFile(filename, overwrite);
}

void STCore::WriteData()
{
  if (fRawEventPtr == NULL) {
    std::cout << "== [STCore] Call this method after GetRawEvent()!" << std::endl;

    return;
  }

  if (fIsSeparatedData)  {
    for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
      for (Int_t iFrame = 0; iFrame < fNumCurrEventFrames[iCobo]; iFrame++) {
        fDecoderPtr[iCobo] -> GetFrame(fCurrEventFrameNo[iCobo][iFrame]);
        fDecoderPtr[iCobo] -> WriteFrame();
      }
    }
  } else 
    fDecoderPtr[0] -> WriteFrame();
}

STRawEvent *STCore::GetRawEvent(Long64_t eventID)
{
  if (!fIsData) {
    std::cout << "== [STCore] Data file is not set!" << std::endl;

    return NULL;
  }

  if (fPedestalMode == kNoPedestal && !fIsPedestalGenerationMode)
    std::cout << "== [STCore] Pedestal data file is not set!" << std::endl;

  if (fIsSeparatedData) {
    if (fCurrEventNo == eventID && fNextEventNo != 0)
      return fRawEventPtr;

    fRawEventPtr -> Clear();
    fPadArray -> Clear("C");

    if (eventID == -1) {
      eventID = fNextEventNo;
    } else if (eventID < fCurrEventNo) {
      fCurrEventNo = eventID;
      for (Int_t iCobo = 0; iCobo < 12; iCobo++)
        fCurrFrameNo[iCobo] = 0;
    }

    fCurrEventNo = eventID;
    fRawEventPtr -> SetEventID(fCurrEventNo);

    fNextEventNo = fCurrEventNo + 1;

    std::thread cobo0([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 0);
    std::thread cobo1([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 1);
    std::thread cobo2([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 2);
    std::thread cobo3([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 3);
    std::thread cobo4([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 4);
    std::thread cobo5([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 5);
    std::thread cobo6([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 6);
    std::thread cobo7([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 7);
    std::thread cobo8([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 8);
    std::thread cobo9([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 9);
    std::thread cobo10([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 10);
    std::thread cobo11([this](Int_t coboIdx) { this -> ProcessCobo(coboIdx); }, 11);

    cobo0.join();
    cobo1.join();
    cobo2.join();
    cobo3.join();
    cobo4.join();
    cobo5.join();
    cobo6.join();
    cobo7.join();
    cobo8.join();
    cobo9.join();
    cobo10.join();
    cobo11.join();

    for (Int_t iRow = 0; iRow < 108; iRow++) {
      for (Int_t iLayer = 0; iLayer < 112; iLayer++) {
        STPad *pad = (STPad *) fPadArray -> At(iRow*112 + iLayer);
        if (pad != NULL)
          fRawEventPtr -> SetPad(pad);
      }
    }

    if (fRawEventPtr -> GetNumPads() == 0 && fRawEventPtr -> IsGood() == kFALSE)
      return NULL; 
    else
      return fRawEventPtr;
  } else {
    fPrevEventNo = eventID;

    fRawEventPtr -> Clear();

    fPadArray -> Clear("C");

    GETFrame *frame = NULL;
    while ((frame = fDecoderPtr[0] -> GetFrame(fCurrFrameNo[0]))) {
      if (fPrevEventNo == -1)
        fPrevEventNo = frame -> GetEventID();

      fCurrEventNo = frame -> GetEventID();

      if (fCurrEventNo == fPrevEventNo + 1) {
        fPrevEventNo = fCurrEventNo;
        return fRawEventPtr;
      } else if (fCurrEventNo > fPrevEventNo + 1) {
        fCurrFrameNo[0] = 0;
        continue;
      } else if (fCurrEventNo < fPrevEventNo) {
        fCurrFrameNo[0]++;
        continue;
      }

      Int_t frameType = fDecoderPtr[0] -> GetFrameType();

      if ((frameType == GETDecoder::kMergedID || frameType == GETDecoder::kMergedTime) && !(fRawEventPtr -> IsGood())) {
        Int_t currentInnerFrameID = fDecoderPtr[0] -> GetCurrentInnerFrameID();
        Int_t numInnerFrames = fDecoderPtr[0] -> GetNumMergedFrames();

        while (!(currentInnerFrameID + 1 == numInnerFrames)) {
          fDecoderPtr[0] -> GetFrame(fCurrFrameNo[0]);

          currentInnerFrameID = fDecoderPtr[0] -> GetCurrentInnerFrameID();
          numInnerFrames = fDecoderPtr[0] -> GetNumMergedFrames();
        }

        fCurrFrameNo[0]++;
        fPrevEventNo = fCurrEventNo;
        return fRawEventPtr;
      }

      fRawEventPtr -> SetEventID(fCurrEventNo);

      Int_t coboID = frame -> GetCoboID();

      if (fOldData == kTRUE && (frameType == GETDecoder::kMergedID || frameType == GETDecoder::kMergedTime))
        coboID = fDecoderPtr[0] -> GetCurrentInnerFrameID();

      Int_t asadID = frame -> GetAsadID();

      for (Int_t iAget = 0; iAget < 4; iAget++) {
        for (Int_t iCh = 0; iCh < 68; iCh++) {
          Int_t row, layer;
          fMapPtr -> GetRowNLayer(coboID, asadID, iAget, iCh, row, layer);

          if (row == -2 || layer == -2)
            continue;

          STPad *pad = new ((*fPadArray)[row*112 + layer]) STPad(row, layer);
          Int_t *rawadc = frame -> GetRawADC(iAget, iCh);
          for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
            pad -> SetRawADC(iTb, rawadc[iTb]);

          if (fPedestalMode == kPedestalInternal) {
            frame -> CalcPedestal(iAget, iCh, fPedestalStartTb, fAverageTbs);
            frame -> SubtractPedestal(iAget, iCh);

            Double_t *adc = frame -> GetADC(iAget, iCh);
            for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
              pad -> SetADC(iTb, adc[iTb]);

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
  //            delete pad;

              iAget = 4;
              iCh = 68;

              continue;
            }

            Double_t *adc = frame -> GetADC(iAget, iCh);

            if (fIsGainCalibrationData)
              fGainCalibrationPtr -> CalibrateADC(row, layer, fNumTbs, adc);

            for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
              pad -> SetADC(iTb, adc[iTb]);

            pad -> SetPedestalSubtracted(kTRUE);
          }

          fRawEventPtr -> SetPad(pad);
  //        delete pad;
        }
      }

      if (frameType == GETDecoder::kMergedID || frameType == GETDecoder::kMergedTime) {
        Int_t currentInnerFrameID = fDecoderPtr[0] -> GetCurrentInnerFrameID();
        Int_t numInnerFrames = fDecoderPtr[0] -> GetNumMergedFrames();

        if (currentInnerFrameID + 1 == numInnerFrames) {
          fCurrFrameNo[0]++;
          fPrevEventNo = fCurrEventNo;
          return fRawEventPtr;
        }
      } else
        fCurrFrameNo[0]++;
    }
  }

  return NULL;
}

Int_t STCore::GetEventID()
{
  return fRawEventPtr -> GetEventID();
}

Int_t STCore::GetNumTbs(Int_t coboIdx)
{
  return fDecoderPtr[coboIdx] -> GetNumTbs();
}

void STCore::SetOldData(Bool_t oldData)
{
  fOldData = oldData;
}

void STCore::SetUseSeparatedData(Bool_t value) {
  fIsSeparatedData = value;

  if (fIsSeparatedData) {
    std::cout << cYELLOW << "== [STCore] You set the decoder to analyze seperated data files." << std::endl;
    std::cout << "            Make sure to call this method right after the instance created!" << cNORMAL << std::endl;

//    fDecoderPtr[0] -> SetDebugMode(1);
    for (Int_t iCobo = 1; iCobo < 12; iCobo++) {
      fDecoderPtr[iCobo] = new GETDecoder();
//      fDecoderPtr[iCobo] -> SetDebugMode(1);
    }
  }
}

STMap *STCore::GetSTMap()
{
  return fMapPtr;
}

STPlot *STCore::GetSTPlot()
{
  if (fPlotPtr == NULL)
    fPlotPtr = new STPlot(this);

  return fPlotPtr;
}
