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

#include "TSystem.h"

#include "STGenerator.hh"

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
  if (mode.EqualTo("pedestal"))
    fMode = kPedestal;
  else if (mode.EqualTo("gain"))
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
STGenerator::StartProcess()
{
  if (fOutputFile.EqualTo("")) {
    cout << "== [STGenerator] Output file is not set!" << endl;

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
    cout << " Data list:" << endl;
    for (Int_t iData = 0; iData < numData; iData++)
      cout << "   " << fCore -> GetDataName(iData) << endl;
  } else if (fMode == kGain) {
    cout << "Gain calibration data generator mode" << endl;
    cout << " Data list:" << endl;
    for (Int_t iData = 0; iData < numData; iData++)
      cout << "   " << Form("%.1f V  ", fVoltageArray.at(iData)) << fCore -> GetDataName(iData) << endl;
  }
  cout << "============================================" << endl;
}
