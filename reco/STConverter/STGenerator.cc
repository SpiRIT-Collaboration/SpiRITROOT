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
#include "TGraph.h"
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
  fIsPositivePolarity = kFALSE;

  fParReader = NULL;
}

STGenerator::STGenerator(TString mode)
{
  fMode = kError;
  SetMode(mode);
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

  Int_t iRow = 0, iLayer = 0;
  Double_t meanBS = 0, sigmaBS = 0, meanAS = 0, sigmaAS = 0;

  TH1D ***beforeSubtractionMean = new TH1D**[fRows];
  TH1D ***beforeSubtractionSigma = new TH1D**[fRows];
  TH1D ***afterSubtractionMean = new TH1D**[fRows];
  TH1D ***afterSubtractionSigma = new TH1D**[fRows];
  for (iRow = 0; iRow < fRows; iRow++) {
    beforeSubtractionMean[iRow] = new TH1D*[fLayers];
    beforeSubtractionSigma[iRow] = new TH1D*[fLayers];
    afterSubtractionMean[iRow] = new TH1D*[fLayers];
    afterSubtractionSigma[iRow] = new TH1D*[fLayers];

    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      beforeSubtractionMean[iRow][iLayer] = new TH1D(Form("mean_bs_%d_%d", iRow, iLayer), "", 4000, 3300, 4100);
      beforeSubtractionSigma[iRow][iLayer] = new TH1D(Form("sigma_bs_%d_%d", iRow, iLayer), "", 1000, 0, 100);
      afterSubtractionMean[iRow][iLayer] = new TH1D(Form("mean_as_%d_%d", iRow, iLayer), "", 4000, -400, 400);
      afterSubtractionSigma[iRow][iLayer] = new TH1D(Form("sigma_as_%d_%d", iRow, iLayer), "", 1000, 0, 100);
    }
  }

  TTree *outTree = new TTree("PedestalData", "Pedestal Data Tree");
  outTree -> Branch("row", &iRow);
  outTree -> Branch("layer", &iLayer);
  outTree -> Branch("meanBS", &meanBS);
  outTree -> Branch("sigmaBS", &sigmaBS);
  outTree -> Branch("meanAS", &meanAS);
  outTree -> Branch("sigmaAS", &sigmaAS);

  GETMath *mathRA = new GETMath();
  GETMath *mathA = new GETMath();
  STRawEvent *event = NULL;
  while ((event = fCore -> GetRawEvent())) {
    Int_t numPads = event -> GetNumPads();

    Int_t eventid = event -> GetEventID();
    if (eventid%100 == 0) 
      cout << "Processing event ID: " << eventid << endl;

    for (Int_t iPad = 0; iPad < numPads; iPad++) {
      STPad *pad = event -> GetPad(iPad);

      Int_t row = pad -> GetRow();
      Int_t layer = pad -> GetLayer();

      Int_t *rawadc = pad -> GetRawADC();
      Double_t *adc = pad -> GetADC();

      mathRA -> Reset();
      mathA -> Reset();
      for (Int_t iTb = 1; iTb < fNumTbs - 1; iTb++) {
        mathRA -> Add(rawadc[iTb]);
        mathA -> Add(adc[iTb]);
      }

      beforeSubtractionMean[row][layer] -> Fill(mathRA -> GetMean());
      beforeSubtractionSigma[row][layer] -> Fill(mathRA -> GetRMS());
      afterSubtractionMean[row][layer] -> Fill(mathA -> GetMean());
      afterSubtractionSigma[row][layer] -> Fill(mathA -> GetRMS());
    }
  }

  cout << "== [STGenerator] Creating pedestal data: " << fOutputFile << endl;
  for (iRow = 0; iRow < fRows; iRow++) {
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      meanBS = beforeSubtractionMean[iRow][iLayer] -> GetMean();
      sigmaBS = beforeSubtractionSigma[iRow][iLayer] -> GetMean();
      meanAS = afterSubtractionMean[iRow][iLayer] -> GetMean();
      sigmaAS = afterSubtractionSigma[iRow][iLayer] -> GetMean();

      outTree -> Fill();
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

      Int_t eventid = event -> GetEventID();
      if (eventid%100 == 0) 
        cout << "Start voltage: " << fVoltageArray.at(iVoltage) << " event: " << eventid << endl;

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

//      cout << "Done voltage: " << fVoltageArray.at(iVoltage) << " event: " << event -> GetEventID() << endl;
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

      TGraph *aPad = new TGraph(numVoltages, means, voltages);
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
