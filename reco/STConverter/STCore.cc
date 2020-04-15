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

#include "GETCoboFrame.hh"
#include "GETLayeredFrame.hh"

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

void STCore::Initialize()
{
  fRawEventPtr = new STRawEvent();

  fMapPtr = new STMap();
  fIsNegativePolarity = kTRUE;
  fPedestalPtr[0] = new STPedestal();
  for (Int_t iCobo = 1; iCobo < 12; iCobo++)
    fPedestalPtr[iCobo] = NULL;

  fGGNoisePtr[0] = new STGGNoiseSubtractor();
  for (Int_t iCobo = 1; iCobo < 12; iCobo++)
    fGGNoisePtr[iCobo] = NULL;

  fIsGGNoiseGenerationMode = kFALSE;
  fIsSetGGNoiseData = kFALSE;

  fPlotPtr = NULL;

  fDecoderPtr[0] = new GETDecoder();
//  fDecoderPtr[0] -> SetDebugMode(1);
  for (Int_t iPad = 0; iPad < 12096; iPad++)
    fPadArray.push_back(new STPad());

  fIsData = kFALSE;
  fFPNSigmaThreshold = 5;

  fGainCalibrationPtr[0] = new STGainCalibration();
  for (Int_t iCobo = 1; iCobo < 12; iCobo++)
    fGainCalibrationPtr[iCobo] = nullptr;

  fIsGainCalibrationData = kFALSE;

  fGainMatchingPtr[0] = new STGainMatching();
  for (Int_t iCobo = 1; iCobo < 12; iCobo++)
    fGainMatchingPtr[iCobo] = nullptr;

  fIsGainMatchingData = kFALSE;

  fNumTbs = 512;

  fTargetFrameID = -1;
  memset(fCurrentEventID, 0, sizeof(Int_t)*12);

  fIsSeparatedData = kFALSE;

  fStartTb = 0;
  fEndTb = fNumTbs;
}

Bool_t STCore::AddData(TString filename, Int_t coboIdx)
{
  return fDecoderPtr[coboIdx] -> AddData(filename);
}

void STCore::SetPositivePolarity(Bool_t value)
{
  fIsNegativePolarity = !value;

  fGGNoisePtr[0] -> SetPolarity(!value);
  for (Int_t iCobo = 1; iCobo < 12; iCobo++)
    fGGNoisePtr[iCobo] -> SetPolarity(!value);
}

Bool_t STCore::SetData(Int_t value)
{
  fIsData = fDecoderPtr[0] -> SetData(value);
  GETDecoder::EFrameType frameType = fDecoderPtr[0] -> GetFrameType();

  if (fIsSeparatedData) {
    for (Int_t iCobo = 1; iCobo < 12; iCobo++) {
      fIsData &= fDecoderPtr[iCobo] -> SetData(value);
      frameType = fDecoderPtr[iCobo] -> GetFrameType();

      if (frameType != GETDecoder::kCobo) {
        std::cout << cRED << "== [STCore] When using separated data, only accepted are not merged frame data files!" << cNORMAL << std::endl;

        fIsData = kFALSE;
        return fIsData;
      }
    }
  }

  fTargetFrameID = -1;
  memset(fCurrentEventID, 0, sizeof(Int_t)*12);

  return fIsData;
}

void STCore::SetDiscontinuousData(Bool_t value)
{
  fDecoderPtr[0] -> SetDiscontinuousData(value);
  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++)
      fDecoderPtr[iCobo] -> SetDiscontinuousData(value);
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
  fGGNoisePtr[0] -> SetNumTbs(value);

  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++) {
      fDecoderPtr[iCobo] -> SetNumTbs(value);
      fGGNoisePtr[iCobo] -> SetNumTbs(value);
    }
}

void STCore::SetFPNPedestal(Double_t sigmaThreshold)
{
  fFPNSigmaThreshold = sigmaThreshold;
  fGGNoisePtr[0] -> SetSigmaThreshold(fFPNSigmaThreshold);
  for (Int_t iCobo = 1; iCobo < 12; iCobo++)
    fGGNoisePtr[iCobo] -> SetSigmaThreshold(fFPNSigmaThreshold);

  std::cout << "== [STCore] Using FPN pedestal is set!" << std::endl;
}

void STCore::SetGGNoiseGenerationMode(Bool_t value) { fIsGGNoiseGenerationMode = value; }

void STCore::SetGGNoiseData(TString ggNoiseData)
{
  fGGNoisePtr[0] -> SetGGNoiseData(ggNoiseData);
  for (Int_t iCobo = 1; iCobo < 12; iCobo++)
    fGGNoisePtr[iCobo] -> SetGGNoiseData(ggNoiseData);

  std::cout << "== [STCore] Gating grid noise data is set!" << std::endl;
}

Bool_t STCore::InitGGNoiseSubtractor()
{
  fIsSetGGNoiseData = fGGNoisePtr[0] -> Init();
  for (Int_t iCobo = 1; iCobo < 12; iCobo++)
    fIsSetGGNoiseData &= fGGNoisePtr[iCobo] -> Init();
  
  std::cout << "== [STCore] Gating grid noise subtractor is initialized! FPN Pedestal subtraction is disabled!" << std::endl;

  return fIsSetGGNoiseData;
}

Bool_t STCore::SetGainCalibrationData(TString filename, TString dataType)
{
  fIsGainCalibrationData = fGainCalibrationPtr[0] -> SetGainCalibrationData(filename, dataType);
  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++)
      fIsGainCalibrationData |= fGainCalibrationPtr[iCobo] -> SetGainCalibrationData(filename, dataType);

  std::cout << "== [STCore] Gain calibration data is set!" << std::endl;
  return fIsGainCalibrationData;
}

void STCore::SetGainReference(Int_t row, Int_t layer)
{
  if (!fIsGainCalibrationData) {
    std::cout << "== [STCore] Set gain calibration data first!" << std::endl;

    return;
  }

  fGainCalibrationPtr[0] -> SetGainReference(row, layer);
  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++)
      fGainCalibrationPtr[iCobo] -> SetGainReference(row, layer);
}

void STCore::SetGainReference(Double_t constant, Double_t linear, Double_t quadratic)
{
  if (!fIsGainCalibrationData) {
    std::cout << "== [STCore] Set gain calibration data first!" << std::endl;

    return;
  }

  fGainCalibrationPtr[0] -> SetGainReference(constant, linear, quadratic);
  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++)
      fGainCalibrationPtr[iCobo] -> SetGainReference(constant, linear, quadratic);
}


Bool_t STCore::SetGainMatchingData(TString filename)
{
  fGainMatchingPtr[0] -> SetDatafile(filename);
  fIsGainMatchingData = fGainMatchingPtr[0] -> Init();
  for (Int_t iCobo = 1; iCobo < 12; iCobo++) {
    fGainMatchingPtr[iCobo] -> SetDatafile(filename);
    fIsGainMatchingData |= fGainMatchingPtr[iCobo] -> Init();
  }

  return fIsGainMatchingData;
}

void STCore::SetTbRange(Int_t startTb, Int_t endTb)
{
  std::cout << "== [STCore] ADC values in the time bucket range [" << startTb << "," << endTb << ") are stored!" << std::endl;

  fStartTb = startTb;
  fEndTb = endTb;
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
  GETCoboFrame *coboFrame = fDecoderPtr[coboIdx] -> GetCoboFrame(fTargetFrameID);

  if (coboFrame == NULL) {
    fRawEventPtr -> SetIsGood(kFALSE);

    return;
  }

  fCurrentEventID[coboIdx] = coboFrame -> GetEventID();
  Int_t numFrames = coboFrame -> GetNumFrames();
  for (Int_t iFrame = 0; iFrame < numFrames; iFrame++) {
    GETBasicFrame *frame = coboFrame -> GetFrame(iFrame);

    Int_t coboID = frame -> GetCoboID();
    Int_t asadID = frame -> GetAsadID();

    for (Int_t iAget = 0; iAget < 4; iAget++) {
      for (Int_t iCh = 0; iCh < 68; iCh++) {
        Int_t row, layer;
        fMapPtr -> GetRowNLayer(coboID, asadID, iAget, iCh, row, layer);

        if (row == -2 || layer == -2)
          continue;

        STPad *pad = fPadArray.at(row*112 + layer);
        pad -> SetRow(row);
        pad -> SetLayer(layer);
        Int_t *rawadc = frame -> GetSample(iAget, iCh);
        for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
          pad -> SetRawADC(iTb, rawadc[iTb]);

        Int_t fpnCh = GetFPNChannel(iCh);
        Double_t adc[512] = {0};
        if (!fIsGGNoiseGenerationMode) {
          if (!fIsSetGGNoiseData)
            fPedestalPtr[coboIdx] -> SubtractPedestal(fNumTbs, frame -> GetSample(iAget, fpnCh), rawadc, adc, fFPNSigmaThreshold);
          else
            fGGNoisePtr[coboIdx] -> SubtractNoise(row, layer, rawadc, adc);
        }

        if (fIsGainCalibrationData)
          fGainCalibrationPtr[coboIdx] -> CalibrateADC(row, layer, fNumTbs, adc);

        if (fIsGainMatchingData)
          fGainMatchingPtr[coboIdx] -> CalibrateADC(layer, row, fNumTbs, adc);

	//mean and std devitation for finding dead pads. dead from saturation of beam
	double mean  = 0.;
	double stdev = 0.;
	double max_adc = 99999.;
        for (Int_t iTb = 0; iTb < fEndTb; iTb++)
	  {
	    if(iTb >= 1)
	      {
		mean = ((iTb-1.)/iTb)*mean + (1./iTb)*rawadc[iTb];
		if(max_adc > rawadc[iTb])//data is negative bias in rawadc i.e. max is minimum in raw
		  max_adc = rawadc[iTb];
	      }
	    if(iTb > 1)
	      stdev = ((iTb - 1.)/iTb)*stdev + (1./(iTb-1))*pow(rawadc[iTb]-mean,2);
	    
	    if( iTb >= fStartTb && iTb < fEndTb)
	      pad -> SetADC(iTb, adc[iTb]);
	  }

	max_adc = mean - max_adc;//pedestal is mean value
	if(sqrt(stdev) < 50 && max_adc < 50)//condition of dead pad
	  {
	    pad -> SetSaturatedTb(0);
	    pad -> SetSaturatedTbMC(0);
	    pad -> SetIsSaturated(true);
	  }
	
        pad -> SetPedestalSubtracted(kTRUE);
      }
    }
  }
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
      fDecoderPtr[iCobo] -> GetCoboFrame(fTargetFrameID);
      fDecoderPtr[iCobo] -> WriteFrame();
    }
  } else 
    fDecoderPtr[0] -> WriteFrame();
}

STRawEvent *STCore::GetRawEvent(Long64_t frameID)
{
  if (!fIsData) {
    std::cout << "== [STCore] Data file is not set!" << std::endl;

    return NULL;
  }

  if (fIsSeparatedData) {
    fRawEventPtr -> Clear();
    for (Int_t iPad = 0; iPad < 12096; iPad++)
      fPadArray.at(iPad) -> Clear();

    if (frameID == -1)
      fTargetFrameID++;
    else
      fTargetFrameID = frameID;

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

    for (Int_t iCobo = 0; iCobo < 12; iCobo++)
      if (fCurrentEventID[0] != fCurrentEventID[iCobo]) {
        std::cout << "== [STCore] Event IDs don't match between CoBos! fCurrentEventID[0]: " << fCurrentEventID[0] << " fCurrentEventID[" << iCobo << "]: " << fCurrentEventID[iCobo] << std::endl;

        fRawEventPtr -> SetIsGood(kFALSE);
      }

    fRawEventPtr -> SetEventID(fCurrentEventID[0]);

    for (Int_t iRow = 0; iRow < 108; iRow++) {
      for (Int_t iLayer = 0; iLayer < 112; iLayer++) {
        STPad *pad = fPadArray.at(iRow*112 + iLayer);
        if (!(pad -> GetRow() == -1 && pad -> GetLayer() == -1))
          fRawEventPtr -> SetPad(pad);
      }
    }

    if (fRawEventPtr -> GetNumPads() == 0 && fRawEventPtr -> IsGood() == kFALSE)
      return NULL; 
    else
      return fRawEventPtr;
  } else {
    fRawEventPtr -> Clear();
    for (Int_t iPad = 0; iPad < 12096; iPad++)
      fPadArray.at(iPad) -> Clear();


    if (frameID == -1)
      fTargetFrameID++;
    else
      fTargetFrameID = frameID;

    GETLayeredFrame *layeredFrame = fDecoderPtr[0] -> GetLayeredFrame(fTargetFrameID);
    if (layeredFrame == NULL)
      return NULL;

    fRawEventPtr -> SetEventID(layeredFrame -> GetEventID());

    Int_t numFrames = layeredFrame -> GetNItems();
    for (Int_t iFrame = 0; iFrame < numFrames; iFrame++) {
      GETBasicFrame *frame = layeredFrame -> GetFrame(iFrame); 

      Int_t coboID = frame -> GetCoboID();
      Int_t asadID = frame -> GetAsadID();

      for (Int_t iAget = 0; iAget < 4; iAget++) {
        for (Int_t iCh = 0; iCh < 68; iCh++) {
          Int_t row, layer;
          fMapPtr -> GetRowNLayer(coboID, asadID, iAget, iCh, row, layer);

          if (row == -2 || layer == -2)
            continue;

          STPad *pad = fPadArray.at(row*112 + layer);
          pad -> SetRow(row);
          pad -> SetLayer(layer);
          Int_t *rawadc = frame -> GetSample(iAget, iCh);
          for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
            pad -> SetRawADC(iTb, rawadc[iTb]);

          Int_t fpnCh = GetFPNChannel(iCh);
          Double_t adc[512] = {0};
          Bool_t good = kFALSE;
          if (!fIsGGNoiseGenerationMode) {
            if (!fIsSetGGNoiseData)
              good = fPedestalPtr[0] -> SubtractPedestal(fNumTbs, frame -> GetSample(iAget, fpnCh), rawadc, adc, fFPNSigmaThreshold);
            else
              fGGNoisePtr[0] -> SubtractNoise(row, layer, rawadc, adc);
          }

          if (fIsGainCalibrationData)
            fGainCalibrationPtr[0] -> CalibrateADC(row, layer, fNumTbs, adc);

          if (fIsGainMatchingData)
            fGainMatchingPtr[0] -> CalibrateADC(layer, row, fNumTbs, adc);

          for (Int_t iTb = fStartTb; iTb < fEndTb; iTb++)
            pad -> SetADC(iTb, adc[iTb]);

          pad -> SetPedestalSubtracted(kTRUE);
          fRawEventPtr -> SetIsGood(good);

          fRawEventPtr -> SetPad(pad);
        }
      }
    }

    return fRawEventPtr;
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

void STCore::SetUseSeparatedData(Bool_t value) {
  fIsSeparatedData = value;

  if (fIsSeparatedData) {
    std::cout << cYELLOW << "== [STCore] You set the decoder to analyze seperated data files." << std::endl;
    std::cout << "            Make sure to call this method right after the instance created!" << cNORMAL << std::endl;

//    fDecoderPtr[0] -> SetDebugMode(1);
    for (Int_t iCobo = 1; iCobo < 12; iCobo++) {
      fDecoderPtr[iCobo] = new GETDecoder();
      fPedestalPtr[iCobo] = new STPedestal();
      fGainCalibrationPtr[iCobo] = new STGainCalibration();
      fGainMatchingPtr[iCobo] = new STGainMatching();
      fGGNoisePtr[iCobo] = new STGGNoiseSubtractor();
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

void STCore::GoToEnd(Int_t coboIdx)
{
  fDecoderPtr[coboIdx] -> GoToEnd();
}

void STCore::GenerateMetaData(Int_t runNo)
{
  if (fIsSeparatedData) {
    std::thread cobo0([this]() { this -> GoToEnd(0); });
    std::thread cobo1([this]() { this -> GoToEnd(1); });
    std::thread cobo2([this]() { this -> GoToEnd(2); });
    std::thread cobo3([this]() { this -> GoToEnd(3); });
    std::thread cobo4([this]() { this -> GoToEnd(4); });
    std::thread cobo5([this]() { this -> GoToEnd(5); });
    std::thread cobo6([this]() { this -> GoToEnd(6); });
    std::thread cobo7([this]() { this -> GoToEnd(7); });
    std::thread cobo8([this]() { this -> GoToEnd(8); });
    std::thread cobo9([this]() { this -> GoToEnd(9); });
    std::thread cobo10([this]() { this -> GoToEnd(10); });
    std::thread cobo11([this]() { this -> GoToEnd(11); });
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

    for (Int_t iCobo = 0; iCobo < 12; iCobo++)
      fDecoderPtr[iCobo] -> SaveMetaData(runNo, "", iCobo);
  } else {
    fDecoderPtr[0] -> GoToEnd();
    fDecoderPtr[0] -> SaveMetaData(runNo);
  }
}

void STCore::LoadMetaData(TString filename, Int_t coboIdx)
{
  if (coboIdx == -1)
    fDecoderPtr[0] -> LoadMetaData(filename);
  else
    fDecoderPtr[coboIdx] -> LoadMetaData(filename);
}

Int_t STCore::GetFPNChannel(Int_t chIdx)
{
  Int_t fpn = -1;

       if (chIdx < 17) fpn = 11;
  else if (chIdx < 34) fpn = 22;
  else if (chIdx < 51) fpn = 45;
  else                 fpn = 56;

  return fpn;
}
