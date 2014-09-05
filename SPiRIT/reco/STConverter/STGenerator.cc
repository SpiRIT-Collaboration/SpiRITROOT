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

#include <iostream>
#include <fstream>

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;

ClassImp(STGenerator)

STGenerator::STGenerator()
{
  fMode = kError;
}

STGenerator::STGenerator(TString mode)
{
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
STGenerator::AddData(TString filename)
{
  if (fMode == kGain) {
    cout << "== [STGenerator] Use AddData(voltage, file) method in Gain mode!" << endl;

    return kFALSE;
  }

  Bool_t okay = fCore -> AddGraw(filename);

  return okay;
}

Bool_t
STGenerator::AddData(Double_t voltage, TString filename)
{
  if (fMode == kPedestal) {
    cout << "== [STGenerator] Use AddData(file) method in Pedestal mode!" << endl;

    return kFALSE;
  }

  Bool_t okay = fCore -> AddGraw(filename);

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
  Double_t ***rms = new Double_t**[fRows];

  for (iRow = 0; iRow < fRows; iRow++) {
    numValues[iRow] = new Int_t*[fLayers];
    mean[iRow] = new Double_t*[fLayers];
    rms[iRow] = new Double_t*[fLayers];
    for (iLayer = 0; iLayer < fLayers; iLayer++) {
      numValues[iRow][iLayer] = new Int_t[fNumTbs];
      mean[iRow][iLayer] = new Double_t[fNumTbs];
      rms[iRow][iLayer] = new Double_t[fNumTbs];

      for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
        numValues[iRow][iLayer][iTb] = 0;
        mean[iRow][iLayer][iTb] = 0;
        rms[iRow][iLayer][iTb] = 0;
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
        math -> Set(numValues[row][layer][iTb]++, mean[row][layer][iTb], rms[row][layer][iTb]);
        math -> Add(adc[iTb]);
        mean[row][layer][iTb] = math -> GetMean();
        rms[row][layer][iTb] = math -> GetRMS();
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
        pedestal[1][iTb] = rms[iRow][iLayer][iTb];
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
