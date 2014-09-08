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

#include "STGenerator.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

#include "GETMath.hh"

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TSpectrum.h"
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
  fIsPersistence = kFALSE;
}

STGenerator::STGenerator(TString mode)
{
  fIsPersistence = kFALSE;
  SetMode(mode);
}

STGenerator::~STGenerator()
{
  if (fCore)
    delete fCore;
}

void
STGenerator::SetMode(TString mode)
{
  if (fMode == kGain || fMode == kPedestal) {
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
  cout << "== [STGenerator] Output file is set to " << filename << "!" << endl;
  cout << "== [STGenerator] Existing file will be overwritten after StartProcess() called!" << endl;

  fOutputFile = filename;
}

Bool_t
STGenerator::SetParameterDir(TString dir)
{
  if (fMode == kError) {
    cerr << "== [STGenerator] Use \"Gain\" or \"Pedestal\" as an argument!" << endl;

    return kFALSE;
  }
  
  TString parameterFile = dir;
  parameterFile.Append("/ST.parameters.par");

  TObjArray *pathElements = 0;
  pathElements = parameterFile.Tokenize("/");

  Int_t numElements = pathElements -> GetLast();

  TString path = "";
  if (numElements == 0)
    path = gSystem -> pwd();
  else {
    for (Int_t i = 0; i < numElements; i++) {
      path.Append(((TObjString *) pathElements -> At(i)) -> GetString());
      path.Append("/");
    }
  }

  TString tempParameterFile = ((TObjString *) pathElements -> Last()) -> GetString();
  delete pathElements;

  parameterFile = gSystem -> Which(path, tempParameterFile);
  if (!parameterFile.EqualTo("")) {
    cout << "== [STGenerator] Parameter file set: " << parameterFile << endl;

    fParameterFile = parameterFile;

    fNumTbs = GetIntParameter("NumTbs");
    fCore -> SetNumTbs(fNumTbs);

    fRows = GetIntParameter("PadRows");
    fLayers = GetIntParameter("PadLayers");
    fPadX = GetIntParameter("PadSizeX");
    fPadZ = GetIntParameter("PadSizeZ");

    Int_t uaMapIndex = GetIntParameter("UAMapFile");
    TString uaMapFile = GetFileParameter(uaMapIndex);
    Bool_t okay = fCore -> SetUAMap(uaMapFile);
    if (okay)
      cout << "== [STGenerator] Unit AsAd mapping file set: " << uaMapFile << endl;
    else {
      cout << "== [STGenerator] Cannot find Unit AsAd mapping file!" << endl;

      return kFALSE;
    }

    Int_t agetMapIndex = GetIntParameter("AGETMapFile");
    TString agetMapFile = GetFileParameter(agetMapIndex);
    okay = fCore -> SetAGETMap(agetMapFile);
    if (okay)
      cout << "== [STGenerator] AGET mapping file set: " << agetMapFile << endl;
    else {
      cout << "== [STGenerator] Cannot find AGET mapping file!" << endl;

      return kFALSE;
    }

    return kTRUE;
  } else {
    cout << "== [STGenerator] Parameter file not found: " << parameterFile << endl;

    return kFALSE;
  }
}

Bool_t
STGenerator::SetPedestalData(TString filename, Int_t startTb, Int_t numTbs)
{
  return fCore -> SetPedestalData(filename, startTb, numTbs);
}

void
STGenerator::SetPersistence(Bool_t value)
{
  fIsPersistence = value;
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
  fNumEvents = numEvents;

  if (fNumEvents < 0)
    cout << "== [STGenerator] Events in passed event list will be excluded!" << endl;
  else if (fNumEvents > 0)
    cout << "== [STGenerator] Events in passed event are processed!" << endl;
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

  if (fMode == kPedestal)
    GeneratePedestal();
  else if (fMode == kGain)
    GenerateGainCalibrationData();
  else
    cout << "== [STGenerator] Notning to do!" << endl;
}

void
STGenerator::GeneratePedestal()
{
  TFile *outFile = new TFile(fOutputFile, "recreate");

  Int_t iRow, iLayer;
  Double_t pedestal[2][512] = {{0}};

  Int_t ***numValues = new Int_t**[fRows];
  Double_t ***mean = new Double_t**[fRows];
  Double_t ***rms2 = new Double_t**[fRows];

  for (iRow = 0; iRow < fRows; iRow++) {
    numValues[iRow] = new Int_t*[fLayers];
    mean[iRow] = new Double_t*[fLayers];
    rms2[iRow] = new Double_t*[fLayers];
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      numValues[iRow][iLayer] = new Int_t[fNumTbs];
      mean[iRow][iLayer] = new Double_t[fNumTbs];
      rms2[iRow][iLayer] = new Double_t[fNumTbs];

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

  STRawEvent *event = NULL;
  Int_t eventID = 0;
  while ((event = fCore -> GetRawEvent())) {
    if (fNumEvents == 0) {}
    else if (fNumEvents > 0) {
      if (eventID == fNumEvents)
        break;
      else if (fEventList[eventID] != event -> GetEventID())
        continue;
    } else if (fNumEvents < 0) {
      if (eventID == -fNumEvents)
        break;
      else if (fEventList[eventID] == event -> GetEventID())
        continue;
    }

    cout << "[STGenerator] Start processing event: " << event -> GetEventID() << endl;

    Int_t numPads = event -> GetNumPads();
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
  Double_t constant, slope;
  
  TFile *outFile = new TFile(fOutputFile, "recreate");
  
  TTree *outTree = new TTree("GainCalibrationData", "Gain Calibration Data Tree");
  outTree -> Branch("padRow", &iRow, "padRow/I");
  outTree -> Branch("padLayer", &iLayer, "padLayer/I");
  outTree -> Branch("constant", &constant, "constant/D");
  outTree -> Branch("slope", &slope, "slope/D");

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

  TSpectrum *peakFinder = new TSpectrum(5);
  Float_t *adcTemp = new Float_t[fNumTbs];
  Float_t *dummy = new Float_t[fNumTbs];

  fCore -> SetNoAutoReload();

  for (Int_t iVoltage = 0; iVoltage < numVoltages; iVoltage++) {
    fCore -> SetData(iVoltage);
    STRawEvent *event = NULL;
    while ((event = fCore -> GetRawEvent())) {
//    for (Int_t iEvent = 0; iEvent < 400; iEvent++) {
//      STRawEvent *event = fCore -> GetRawEvent();

      cout << "Start voltage: " << fVoltageArray.at(iVoltage) << " event: " << event -> GetEventID() << endl;

      Int_t numPads = event -> GetNumPads();
      for (Int_t iPad = 0; iPad < numPads; iPad++) {
        STPad *pad = event -> GetPad(iPad);
        Double_t *adc = pad -> GetADC();

        for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
          adcTemp[iTb] = adc[iTb];

        Int_t row = pad -> GetRow();
        Int_t layer = pad -> GetLayer();

        Int_t numPeaks = peakFinder -> SearchHighRes(adcTemp, dummy, fNumTbs, 4.7, 90, kFALSE, 3, kTRUE, 3);

        if (numPeaks != 1)
          cout << row << " " << layer << " " << numPeaks << " " << (Int_t)ceil((peakFinder -> GetPositionX())[0]) << " " << adc[(Int_t)ceil((peakFinder -> GetPositionX())[0])] << endl;
        Double_t max = adc[(Int_t)ceil((peakFinder -> GetPositionX())[0])];

        padHist[row][layer][iVoltage] -> Fill(max);
      }

      cout << "Done voltage: " << fVoltageArray.at(iVoltage) << " event: " << event -> GetEventID() << endl;
    }
  }

  // C: Constant(0), S: Slope(1) 
  Double_t ***fCS = new Double_t**[fRows];
  for (iRow = 0; iRow < fRows; iRow++) {
    fCS[iRow] = new Double_t*[fLayers];
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      fCS[iRow][iLayer] = new Double_t[2];

      fCS[iRow][iLayer][0] = 0;
      fCS[iRow][iLayer][1] = 0;
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

        TSpectrum spectrum;
        spectrum.Search(thisHist, 30, "goff", 0.7);
        Double_t min = (spectrum.GetPositionX())[0] - 80; 
        Double_t max = (spectrum.GetPositionX())[0] + 80; 

        thisHist -> Fit("gaus", "0", "", min, max);
        if (!(thisHist -> GetFunction("gaus")))
          continue;

        means[iVoltage] = ((TF1 *) thisHist -> GetFunction("gaus")) -> GetParameter(1);
        sigmas[iVoltage] = ((TF1 *) thisHist -> GetFunction("gaus")) -> GetParameter(2);
      }

      if (empty)
        continue;

      Int_t isFail = kTRUE;

      if (fIsPersistence) {
        TGraphErrors *aPad = new TGraphErrors(numVoltages, voltages, means, 0, sigmas);
        aPad -> SetName(Form("pad_%d_%d", iRow, iLayer));
        aPad -> Fit("pol1", "0");

        fCS[iRow][iLayer][0] = ((TF1 *) aPad -> GetFunction("pol1")) -> GetParameter(0);
        fCS[iRow][iLayer][1] = ((TF1 *) aPad -> GetFunction("pol1")) -> GetParameter(1);

        isFail = kFALSE;
        aPad -> Write();
      } else {
        TGraphErrors aPad(numVoltages, voltages, means, 0, sigmas);
        aPad.LeastSquareLinearFit(numVoltages, fCS[iRow][iLayer][0], fCS[iRow][iLayer][1], isFail);
      }

      if (isFail) {
        cerr << "Error when fit!" << endl;

        return;
      }
    }
  }

  delete means;
  delete sigmas;

  cout << "== Creating gain calibration data: " << fOutputFile << endl;

  for (iRow = 0; iRow < fRows; iRow++) {
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      constant = fCS[iRow][iLayer][0];
      slope = fCS[iRow][iLayer][1];

      outTree -> Fill();
    }
  }

  if (!fIsPersistence)
    delete padHist;

  outFile -> Write();
  cout << "== Gain calibration data " << fOutputFile << " Created!" << endl;
}

Int_t
STGenerator::GetIntParameter(TString parameter)
{
  ifstream parameters(fParameterFile);
  while (kTRUE) {
    TString value;
    value.ReadToken(parameters);
    if (value.EqualTo(Form("%s:Int_t", parameter.Data()))) {
      value.ReadToken(parameters);
      parameters.close();
      return value.Atoi();
    }
  }
  parameters.close();
}

TString
STGenerator::GetFileParameter(Int_t index)
{
  TString listFile = fParameterFile;
  listFile.ReplaceAll("ST.parameters.par", "ST.files.par");

  ifstream fileList(listFile.Data());

  Char_t buffer[256];
  for (Int_t iFileNum = 0; iFileNum < index + 1; iFileNum++) {
    if (fileList.eof()) {
      cout << "== [STGenerator] Cannot find string #" << iFileNum << endl;

      return "";
    }

    fileList.getline(buffer, 256);
  }

  fileList.close();

  return listFile.ReplaceAll("parameters/ST.files.par", buffer);
}

void
STGenerator::Print()
{
  if (fMode == kError) {
    cerr << "== [STGenerator] Use \"Gain\" or \"Pedestal\" as an argument!" << endl;

    return;
  }

  Int_t numData = fCore -> GetNumData();

  cout << "============================================" << endl;
  cout << " Mode: ";
  if (fMode == kPedestal) {
    cout << "Pedestal data generator mode" << endl;
    cout << " Output File: " << fOutputFile << endl;
    cout << " Data list:" << endl;
    for (Int_t iData = 0; iData < numData; iData++)
      cout << "   " << fCore -> GetDataName(iData) << endl;
  } else if (fMode == kGain) {
    cout << "Gain calibration data generator mode" << endl;
    cout << " Output File: " << fOutputFile << endl;
    cout << " Data list:" << endl;
    for (Int_t iData = 0; iData < numData; iData++)
      cout << "   " << Form("%.1f V  ", fVoltageArray.at(iData)) << fCore -> GetDataName(iData) << endl;
  }
  cout << "============================================" << endl;
}
