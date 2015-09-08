// =================================================
//  STGenerator Class
// 
//  Description:
//    Using the pedestal run data or pulser run data
//    as its input, generates input data for pedestal
//    subtraction or gain calibration data.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 09. 05
// =================================================

#include "STGlobal.hh"
#include "STGenerator.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

#include "GETMath.hh"

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TF1.h"

#include <iostream>
#include <fstream>
#include <cmath>

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;

ClassImp(STGenerator)

STGenerator::STGenerator()
{
  fMode = kError;
  fIsStoreRMS = kFALSE;
  fIsPositivePolarity = kFALSE;

  fParReader = NULL;
}

STGenerator::STGenerator(TString mode)
{
  fMode = kError;
  SetMode(mode);
  fIsStoreRMS = kFALSE;
  fIsPositivePolarity = kFALSE;

  fParReader = NULL;
}

STGenerator::~STGenerator()
{
  if (fCore)
    delete fCore;
}

void
STGenerator::SetPositivePolarity(Bool_t value)
{
  fIsPositivePolarity = value;
}

void
STGenerator::SetMode(TString mode)
{
  if (fMode != kError) {
    cerr << "== [STGenerator] Mode is already set to ";
    if (fMode == kPedestal)
      cerr << "Pedestal!";
    else if (fMode == kGain)
      cerr << "Gain!";
    cout << endl;
    cerr << "== [STGenerator] Create another instance to use the other mode!" << endl;

    return;
  }

  mode.ToLower();
  if (mode.EqualTo("pedestal")) {
    fMode = kPedestal;
    fNumEvents = 0;
    fSumRMSCut = 0;
  } else if (mode.EqualTo("gain"))
    fMode = kGain;
  else {
    fMode = kError;

    cerr << "== [STGenerator] Use \"Gain\" or \"Pedestal\" as an argument!" << endl;

    return;
  }

  fCore = new STCore();
}

void
STGenerator::SetOutputFile(TString filename)
{
  fOutputFile = filename;

  cout << "== [STGenerator] Output file is set to " << filename << "!" << endl;
  if (fMode == kGain) {
    filename.ReplaceAll(".root", ".checking.root");
    cout << "== [STGenerator] Checking file is generated with " << filename << "!" << endl;
  }
  cout << "== [STGenerator] Existing file will be overwritten after StartProcess() called!" << endl;
}

Bool_t
STGenerator::SetParameterFile(TString filename)
{
  TString parameterFile = filename;

  if (fParReader != NULL)
    delete fParReader;

  fParReader = new STParReader(parameterFile);
  if (!(fParReader -> IsGood()))
    return kFALSE;

  fNumTbs = fParReader -> GetIntPar("NumTbs");
  fCore -> SetNumTbs(fNumTbs);

  fRows = fParReader -> GetIntPar("PadRows");
  fLayers = fParReader -> GetIntPar("PadLayers");
  fPadX = fParReader -> GetDoublePar("PadSizeX");
  fPadZ = fParReader -> GetDoublePar("PadSizeZ");

  Int_t uaMapIndex = fParReader -> GetIntPar("UAMapFile");
  TString uaMapFile = fParReader -> GetFilePar(uaMapIndex);
  Bool_t okay = fCore -> SetUAMap(uaMapFile);
  if (okay)
    cout << "== [STGenerator] Unit AsAd mapping file set: " << uaMapFile << endl;
  else {
    cout << "== [STGenerator] Cannot find Unit AsAd mapping file!" << endl;

    return kFALSE;
  }

  Int_t agetMapIndex = fParReader -> GetIntPar("AGETMapFile");
  TString agetMapFile = fParReader -> GetFilePar(agetMapIndex);
  okay = fCore -> SetAGETMap(agetMapFile);
  if (okay)
    cout << "== [STGenerator] AGET mapping file set: " << agetMapFile << endl;
  else {
    cout << "== [STGenerator] Cannot find AGET mapping file!" << endl;

    return kFALSE;
  }

  return kTRUE;
}

void
STGenerator::SetInternalPedestal(Int_t startTb, Int_t numTbs)
{
  fCore -> SetInternalPedestal(startTb, numTbs);
}

Bool_t
STGenerator::SetPedestalData(TString filename, Double_t rmsFactor)
{
  return fCore -> SetPedestalData(filename, rmsFactor);
}

void
STGenerator::SetFPNPedestal(Double_t fpnThreshold)
{
  return fCore -> SetFPNPedestal(fpnThreshold);
}

void
STGenerator::SetStoreRMS(Bool_t value)
{
  if (fMode == kGain) {
    cout << "== [STGenerator::SetStoreRMS()] This method only valid with Pedestal data generation mode!" << endl;

    return;
  }

  fIsStoreRMS = value;
}

void
STGenerator::SetSumRMSCut(Int_t value)
{
  fSumRMSCut = value;
}

Bool_t
STGenerator::AddData(TString filename)
{
  if (fMode == kGain) {
    cout << "== [STGenerator] Use AddData(voltage, file) method in Gain mode!" << endl;

    return kFALSE;
  }

  Bool_t okay = fCore -> AddData(filename);

  return okay;
}

Bool_t
STGenerator::AddData(Double_t voltage, TString filename)
{
  if (fMode == kPedestal) {
    cout << "== [STGenerator] Use AddData(file) method in Pedestal mode!" << endl;

    return kFALSE;
  }

  Bool_t okay = fCore -> AddData(filename);

  if (okay)
    fVoltageArray.push_back(voltage);

  return okay;
}

Bool_t
STGenerator::SetData(Int_t index)
{
  return fCore -> SetData(index);
}

void
STGenerator::SelectEvents(Int_t numEvents, Int_t *eventList)
{
  if (fMode == kGain) {
    cout << "== [STGenerator::SelectEvents()] This method only valid with Pedestal data generation mode!" << endl;

    return;
  }

  fNumEvents = numEvents;

  if (fNumEvents < 0)
    cout << "== [STGenerator] Events in passed event list will be excluded until event ID " << -fNumEvents << "!" << endl;
  else if (fNumEvents > 0)
    cout << "== [STGenerator] Events in passed event list are processed!" << endl;
  else {
    cout << "== [STGenerator] All events are processed!" << endl;

    return;
  }

  fEventList = eventList;
}

void
STGenerator::StartProcess()
{
  if (fOutputFile.EqualTo("")) {
    cout << "== [STGenerator] Output file is not set!" << endl;

    return;
  }

  fCore -> SetPositivePolarity(fIsPositivePolarity);

  if (fMode == kPedestal) {
    fCore -> SetData(0);
    fCore -> SetPedestalGenerationMode();

    GeneratePedestalData();
  }
  else if (fMode == kGain)
    GenerateGainCalibrationData();
  else
    cout << "== [STGenerator] Notning to do!" << endl;
}

void
STGenerator::GeneratePedestalData()
{
  TFile *outFile = new TFile(fOutputFile, "recreate");

  Int_t iRow, iLayer;
  Double_t pedestal[2][512] = {{0}};

  Int_t ***numValues = new Int_t**[fRows];
  Double_t ***mean = new Double_t**[fRows];
  Double_t ***rms2 = new Double_t**[fRows];

  Double_t **padMean = new Double_t*[fRows];
  Double_t **padRMS2 = new Double_t*[fRows];

  for (iRow = 0; iRow < fRows; iRow++) {
    numValues[iRow] = new Int_t*[fLayers];
    mean[iRow] = new Double_t*[fLayers];
    rms2[iRow] = new Double_t*[fLayers];

    padMean[iRow] = new Double_t[fLayers];
    padRMS2[iRow] = new Double_t[fLayers];
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      numValues[iRow][iLayer] = new Int_t[fNumTbs];
      mean[iRow][iLayer] = new Double_t[fNumTbs];
      rms2[iRow][iLayer] = new Double_t[fNumTbs];

      padMean[iRow][iLayer] = 0;
      padRMS2[iRow][iLayer] = 0;
      for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
        numValues[iRow][iLayer][iTb] = 0;
        mean[iRow][iLayer][iTb] = 0;
        rms2[iRow][iLayer][iTb] = 0;
      }
    }
  }
  
  GETMath *math = new GETMath();

  /*
  GETMath ****math = new GETMath***[fRows];
  for (iRow = 0; iRow < fRows; iRow++) {
    math[iRow] = new GETMath**[fLayers];
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      math[iRow][iLayer] = new GETMath*[fNumTbs];
      for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
        math[iRow][iLayer][iTb] = new GETMath();
      }
    }
  }
  */

  Double_t rmsSum = 0;
  Int_t storeEventID = 0;
  Int_t excluded = 0;

  TTree *rmsTree = NULL;
  if (fIsStoreRMS) {
    rmsTree = new TTree("rmsSumTree", "");
    rmsTree -> Branch("eventID", &storeEventID, "eventID/I");
    rmsTree -> Branch("rmsSum", &rmsSum, "rmsSum/D");
    rmsTree -> Branch("excluded", &excluded, "excluded/I");
  }

  STRawEvent *event = NULL;
  Int_t eventID = 0;
  Int_t numExcluded = 0;
  while ((event = fCore -> GetRawEvent())) {
    if (fNumEvents == 0) {}
    else if (fNumEvents > 0) {
      if (eventID == fNumEvents)
        break;
      else if (fEventList[eventID] != event -> GetEventID())
        continue;
    } else if (fNumEvents < 0) {
      if (numExcluded < -fNumEvents && fEventList[numExcluded] == event -> GetEventID()) {
        numExcluded++;
        continue;
      }
    }

    rmsSum = 0;
    excluded = 0;

    Int_t numPads = event -> GetNumPads();

    for (Int_t iPad = 0; iPad < numPads; iPad++) {
      STPad *pad = event -> GetPad(iPad);
      Int_t *adc = pad -> GetRawADC();

      Int_t row = pad -> GetRow();
      Int_t layer = pad -> GetLayer();

      Int_t numTbs = 0;
      for (Int_t iTb = 3; iTb < fNumTbs - 3; iTb++) {
        math -> Reset();
        math -> Set(numTbs++, padMean[row][layer], padRMS2[row][layer]);
        math -> Add(adc[iTb]);
        padMean[row][layer] = math -> GetMean();
        padRMS2[row][layer] = math -> GetRMS2();
      }

      rmsSum += sqrt(padRMS2[row][layer]);
    }

    if (fSumRMSCut != 0 && rmsSum > fSumRMSCut)
      excluded = 1;

    if (fIsStoreRMS) {
      storeEventID = event -> GetEventID();
      rmsTree -> Fill();
    }

    if (excluded) {
      if (event -> GetEventID()%200 == 0)
        cout << "[STGenerator] Skip event: " << event -> GetEventID() << endl;

      continue;
    }

    if (event -> GetEventID()%200 == 0)
      cout << "[STGenerator] Start processing event: " << event -> GetEventID() << endl;

    for (Int_t iPad = 0; iPad < numPads; iPad++) {
      STPad *pad = event -> GetPad(iPad);
      Int_t *adc = pad -> GetRawADC();

      Int_t row = pad -> GetRow();
      Int_t layer = pad -> GetLayer();

      for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
        math -> Reset();
        math -> Set(numValues[row][layer][iTb]++, mean[row][layer][iTb], rms2[row][layer][iTb]);
        math -> Add(adc[iTb]);
        mean[row][layer][iTb] = math -> GetMean();
        rms2[row][layer][iTb] = math -> GetRMS2();

        /*
        math[row][layer][iTb] -> Add(adc[iTb]);
        */
      }
    }

    if (event -> GetEventID()%200 == 0)
      cout << "[STGenerator] Done processing event: " << event -> GetEventID() << endl;

    eventID++;
  }

  cout << "== [STGenerator] Creating pedestal data: " << fOutputFile << endl;
  TTree *tree = new TTree("PedestalData", "Pedestal Data Tree");
  tree -> Branch("padRow", &iRow, "padRow/I");
  tree -> Branch("padLayer", &iLayer, "padLayer/I");
  tree -> Branch("pedestal", &pedestal[0], Form("pedestal[%d]/D", fNumTbs));
  tree -> Branch("pedestalSigma", &pedestal[1], Form("pedestalSigma[%d]/D", fNumTbs));

  for (iRow = 0; iRow < fRows; iRow++) {
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
        pedestal[0][iTb] = mean[iRow][iLayer][iTb];
        pedestal[1][iTb] = sqrt(rms2[iRow][iLayer][iTb]);
        
        /*
        pedestal[0][iTb] = math[iRow][iLayer][iTb] -> GetMean();
        pedestal[1][iTb] = math[iRow][iLayer][iTb] -> GetRMS();
        */
      }

      tree -> Fill();
    }
  }

  outFile -> Write();
  delete outFile;

  cout << "== [STGenerator] Pedestal data " << fOutputFile << " Created!" << endl;
}

void
STGenerator::GenerateGainCalibrationData()
{
  Int_t numVoltages = fVoltageArray.size();

  Int_t iRow, iLayer;
  // Quadratic fit
  Double_t constant, linear, quadratic;
  
  TFile *outFile = new TFile(fOutputFile, "recreate");
  
  TTree *outTree = new TTree("GainCalibrationData", "Gain Calibration Data Tree");
  outTree -> Branch("padRow", &iRow);
  outTree -> Branch("padLayer", &iLayer);
  outTree -> Branch("constant", &constant);
  outTree -> Branch("linear", &linear);
  outTree -> Branch("quadratic", &quadratic);

  TString checkingFilename = fOutputFile;
  checkingFilename.ReplaceAll(".root", ".checking.root");
  TFile *checkingFile = new TFile(checkingFilename, "recreate");

  TH1D ****padHist = new TH1D***[fRows];
  for (iRow = 0; iRow < fRows; iRow++) {
    padHist[iRow] = new TH1D**[fLayers];
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      padHist[iRow][iLayer] = new TH1D*[numVoltages];
      for (Int_t iVoltage = 0; iVoltage < numVoltages; iVoltage++) {
        padHist[iRow][iLayer][iVoltage] = new TH1D(Form("hist_%d_%d_%d", iRow, iLayer, iVoltage), "", 4096, 0, 4096);
      }
    }
  }

  fCore -> SetNoAutoReload();

  for (Int_t iVoltage = 0; iVoltage < numVoltages; iVoltage++) {
    fCore -> SetData(iVoltage);
    STRawEvent *event = NULL;
    while ((event = fCore -> GetRawEvent())) {
//    for (Int_t iEvent = 0; iEvent < 10; iEvent++) {
//      STRawEvent *event = fCore -> GetRawEvent();

      cout << "Start voltage: " << fVoltageArray.at(iVoltage) << " event: " << event -> GetEventID() << endl;

      Int_t numPads = event -> GetNumPads();
      for (Int_t iPad = 0; iPad < numPads; iPad++) {
        STPad *pad = event -> GetPad(iPad);
        Double_t *adc = pad -> GetADC();

        Int_t row = pad -> GetRow();
        Int_t layer = pad -> GetLayer();

        Double_t max = -10;
        for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
          if (adc[iTb] > max)
            max = adc[iTb];
        }

        padHist[row][layer][iVoltage] -> Fill(max);
      }

      cout << "Done voltage: " << fVoltageArray.at(iVoltage) << " event: " << event -> GetEventID() << endl;
    }
  }

  Double_t *voltages = new Double_t[numVoltages];
  for (Int_t iVoltage = 0; iVoltage < numVoltages; iVoltage++)
    voltages[iVoltage] = fVoltageArray.at(iVoltage);

  Double_t *means = new Double_t[numVoltages];
  Double_t *sigmas = new Double_t[numVoltages];

  for (iRow = 0; iRow < fRows; iRow++) {
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      Bool_t empty = kFALSE;

      for (Int_t iVoltage = 0; iVoltage < numVoltages; iVoltage++) {
        means[iVoltage] = 0;
        sigmas[iVoltage] = 0;

        TH1D *thisHist = padHist[iRow][iLayer][iVoltage];
        if (thisHist -> GetEntries() == 0) {
          empty |= kTRUE;
          break;
        }

        means[iVoltage] = thisHist -> GetMean();
        sigmas[iVoltage] = thisHist -> GetRMS();
      }

      if (empty)
        continue;

      Int_t isFail = kTRUE;

#ifdef VVSADC
      Double_t *dummyError = new Double_t[numVoltages];
      for (Int_t iVoltage = 0; iVoltage < numVoltages; iVoltage++)
        dummyError[iVoltage] = 1.E-5;

      TGraphErrors *aPad = new TGraphErrors(numVoltages, means, voltages, sigmas, dummyError);
#else
      TGraphErrors *aPad = new TGraphErrors(numVoltages, voltages, means, 0, sigmas);
#endif
      aPad -> SetName(Form("pad_%d_%d", iRow, iLayer));
      aPad -> Fit("pol2", "0Q");

      constant = ((TF1 *) aPad -> GetFunction("pol2")) -> GetParameter(0);
      linear = ((TF1 *) aPad -> GetFunction("pol2")) -> GetParameter(1);
      quadratic = ((TF1 *) aPad -> GetFunction("pol2")) -> GetParameter(2);

      isFail = kFALSE;

      outTree -> Fill();

      checkingFile -> cd(); 
      aPad -> Write();

      if (isFail) {
        cerr << "Error when fit!" << endl;

        return;
      }
    }
  }

  delete means;
  delete sigmas;
  delete padHist;

  outFile -> Write();
  checkingFile -> Write();
  cout << "== Gain calibration data " << fOutputFile << " Created!" << endl;

  fOutputFile.ReplaceAll(".root", ".checking.root");
  cout << "== Gain calibration checking data " << fOutputFile << " Created!" << endl;
}

void
STGenerator::Print()
{
  if (fMode == kError) {
    cerr << "== [STGenerator] Nothing to print!" << endl;

    return;
  }

  Int_t numData = fCore -> GetNumData();

  cout << "============================================" << endl;
  cout << " Mode: ";
  if (fMode == kPedestal) {
    cout << "Pedetal data generator mode" << endl;
    cout << " Output File: " << fOutputFile << endl;
    cout << " Data list:" << endl;
    for (Int_t iData = 0; iData < numData; iData++)
      cout << "   " << fCore -> GetDataName(iData) << endl;
  } else if (fMode == kGain) {
    cout << "Gain calibration data generator mode" << endl;
    cout << " Output File: " << fOutputFile << endl;
    cout << " Data list:" << endl;
    for (Int_t iData = 0; iData < numData; iData++)
      cout << "   " << Form("%.2f V  ", fVoltageArray.at(iData)) << fCore -> GetDataName(iData) << endl;
  }
  cout << "============================================" << endl;
}
