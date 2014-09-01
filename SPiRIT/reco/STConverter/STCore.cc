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
  AddGraw(filename);
}

STCore::STCore(TString filename, Int_t numTbs)
{
  Initialize();
  AddGraw(filename);
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

  fIsGraw = kFALSE;
  fIsPedestalData = kFALSE;
  fIsInternalPedestal = kFALSE;

  fGainCalibrationPtr = new STGainCalibration();
  fIsGainCalibrationData = kFALSE;

  fSignalDelayPtr = new STSignalDelay();
  fIsSignalDelayData = kFALSE;

  fStartTb = 3;
  fNumTbs = 20;

  fPrevEventNo = -1;
  fCurrEventNo = -1;
  fCurrFrameNo = 0;
}

void STCore::AddGraw(TString filename)
{
  fDecoderPtr -> AddGraw(filename);
  fIsGraw = fDecoderPtr -> SetData(0);
}

void STCore::SetNumTbs(Int_t value)
{
  fDecoderPtr -> SetNumTbs(value);
}

void STCore::SetInternalPedestal(Int_t startTb, Int_t numTbs)
{
  fIsInternalPedestal = kTRUE;
  fIsPedestalData = kFALSE;

  fStartTb = startTb;
  fNumTbs = numTbs;
}

Bool_t STCore::SetPedestalData(TString filename, Int_t startTb, Int_t numTbs)
{
  fIsPedestalData = fPedestalPtr -> SetPedestalData(filename);

  if (fIsPedestalData) {
    fIsInternalPedestal = kFALSE;

    fStartTb = startTb;
    fNumTbs = numTbs;
  } else
    std::cout << "== Pedestal data is not set! Check it exists!" << std::endl;

  return fIsPedestalData;
}

Bool_t STCore::SetGainCalibrationData(TString filename)
{
  fIsGainCalibrationData = fGainCalibrationPtr -> SetGainCalibrationData(filename);

  return fIsGainCalibrationData;
}

Bool_t STCore::SetSignalDelayData(TString filename)
{
  fIsSignalDelayData = fSignalDelayPtr -> SetSignalDelayData(filename);

  return fIsSignalDelayData;
}


void STCore::SetUAMap(TString filename)
{
  fMapPtr -> SetUAMap(filename);
}

void STCore::SetAGETMap(TString filename)
{
  fMapPtr -> SetAGETMap(filename);
}

STRawEvent *STCore::GetRawEvent(Int_t eventID)
{
  if (!fIsGraw) {
    std::cout << "== Graw file is not set!" << std::endl;

    return NULL;
  }

  if (!fIsPedestalData && !fIsInternalPedestal) {
    std::cout << "== Pedestal data file is not set!" << std::endl;
  }

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

    Int_t coboID = fDecoderPtr -> GetCurrentInnerFrameID();
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

        if (fIsInternalPedestal) {
          frame -> CalcPedestal(iAget, iCh, fStartTb, fNumTbs);
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
        } else if (fIsPedestalData) {
          frame -> CalcPedestal(iAget, iCh, fStartTb, fNumTbs);

          Double_t pedestal[512];
          Double_t pedestalSigma[512];

          fPedestalPtr -> GetPedestal(row, layer, pedestal, pedestalSigma);
          frame -> SetPedestal(iAget, iCh, pedestal, pedestalSigma);
          frame -> SubtractPedestal(iAget, iCh);

          Double_t scaleFactor = 1;
          if (fIsGainCalibrationData) {
            scaleFactor = fGainCalibrationPtr -> GetScaleFactor(row, layer);
            pad -> SetGainCalibrated(kTRUE);
          }

          Double_t *adc = frame -> GetADC(iAget, iCh);
          for (Int_t iTb = 0; iTb < fDecoderPtr -> GetNumTbs(); iTb++) {
            Int_t delayedTb = iTb - signalDelay;
            Double_t delayedADC = 0;
            if (delayedTb < 0 || delayedTb >= fDecoderPtr -> GetNumTbs())
              delayedADC = 0;
            else
              delayedADC = adc[delayedTb]*scaleFactor;

            pad -> SetADC(iTb, delayedADC);
          }

          Int_t maxADCIdx = frame -> GetMaxADCIdx(iAget, iCh) + signalDelay;
          if (maxADCIdx < 0 || maxADCIdx >= fDecoderPtr -> GetNumTbs())
            maxADCIdx = 0;

          pad -> SetMaxADCIdx(maxADCIdx);
          pad -> SetPedestalSubtracted(kTRUE);
        }

        fRawEventPtr -> SetPad(pad);
      }
    }

    Int_t frameType = fDecoderPtr -> GetFrameType();
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
