/**
* @brief Store parameters from ST.parameters.par for later use.
*/

#include "STDigiPar.hh"
#include <iostream>
#include <fstream>

ClassImp(STDigiPar)

STDigiPar::STDigiPar(const Char_t *name, const Char_t *title, const Char_t *context)
: FairParGenericSet("STDigiPar", "LAMPS TPC Parameter Container", "")
{
  fInitialized = kFALSE;
  fLogger = FairLogger::GetLogger();
}

STDigiPar::~STDigiPar()
{
}

Double_t STDigiPar::GetPadPlaneX()                   { return fPadPlaneX; }
Double_t STDigiPar::GetPadPlaneZ()                   { return fPadPlaneZ; }
Double_t STDigiPar::GetPadSizeX()                    { return fPadSizeX; }
Double_t STDigiPar::GetPadSizeZ()                    { return fPadSizeZ; }
Int_t    STDigiPar::GetPadRows()                     { return fPadRows; }
Int_t    STDigiPar::GetPadLayers()                   { return fPadLayers; }
Double_t STDigiPar::GetAnodeWirePlaneY()             { return fAnodeWirePlaneY; }
Double_t STDigiPar::GetGroundWirePlaneY()            { return fGroundWirePlaneY; }
Double_t STDigiPar::GetGatingWirePlaneY()            { return fGatingWirePlaneY; }
Double_t STDigiPar::GetEField()                      { return fEField; }
Double_t STDigiPar::GetFPNPedestalRMS()              { return fFPNPedestalRMS; }
Int_t    STDigiPar::GetNumTbs()                      { return fNumTbs; }
Int_t    STDigiPar::GetWindowNumTbs()                { return fWindowNumTbs; }
Int_t    STDigiPar::GetWindowStartTb()               { return fWindowStartTb; }
Int_t    STDigiPar::GetSamplingRate()                { return fSamplingRate; }
Double_t STDigiPar::GetDriftLength()                 { return fDriftLength; }
Int_t    STDigiPar::GetYDivider()                    { return fYDivider; }
Double_t STDigiPar::GetEIonize()                     { return fEIonize; }
Double_t STDigiPar::GetDriftVelocity()               { return fDriftVelocity; }
Double_t STDigiPar::GetCoefDiffusionLong()           { return fCoefDiffusionLong; }
Double_t STDigiPar::GetCoefDiffusionTrans()          { return fCoefDiffusionTrans; }
Double_t STDigiPar::GetCoefAttachment()              { return fCoefAttachment; }
Int_t    STDigiPar::GetGain()                        { return fGain; }
TString  STDigiPar::GetTrackingParFileName()         { return fTrackingParFileName; }
TString  STDigiPar::GetUAMapFileName()               { return fUAMapFileName; }
TString  STDigiPar::GetAGETMapFileName()             { return fAGETMapFileName; }
TString  STDigiPar::GetGainCalibrationDataFileName() { return fGainCalibrationDataFileName; }
Double_t STDigiPar::GetGCConstant()                  { return fGCConstant; }
Double_t STDigiPar::GetGCLinear()                    { return fGCLinear; }
Double_t STDigiPar::GetGCQuadratic()                 { return fGCQuadratic; }
Bool_t   STDigiPar::IsEmbed()                        { return fIsEmbed; }
Double_t STDigiPar::GetYDriftOffset()                { return fYDriftOffset; }
Double_t STDigiPar::GetTotalADCWhenSat()               { return fTotalADCWhenSat; }

void     STDigiPar::SetIsEmbed(Bool_t val)               { fIsEmbed = val; }

Bool_t 
STDigiPar::getParams(FairParamList *paramList)
{
  if (!paramList) {
    fLogger -> Fatal(MESSAGE_ORIGIN, "Parameter list doesn't exist!");
    return kFALSE;
  }

  if (!fInitialized) {
    if (!(paramList -> fill("PadPlaneX", &fPadPlaneX))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadPlaneX parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadPlaneZ", &fPadPlaneZ))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadPlaneZ parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadSizeX", &fPadSizeX))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadSizeX parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadSizeZ", &fPadSizeZ))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadSizeZ parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadRows", &fPadRows))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadRows parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadLayers", &fPadLayers))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadLayers parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("AnodeWirePlaneY", &fAnodeWirePlaneY))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find AnodeWirePlaneY parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("GroundWirePlaneY", &fGroundWirePlaneY))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find GroundWirePlaneY parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("GatingWirePlaneY", &fGatingWirePlaneY))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find GatingWirePlaneY parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("FPNPedestalRMS", &fFPNPedestalRMS))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find FPNPedestalRMS parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("EField", &fEField))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find EField parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("NumTbs", &fNumTbs))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find NumTbs parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("WindowNumTbs", &fWindowNumTbs))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find WindowNumTbs parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("WindowStartTb", &fWindowStartTb))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find WindowStartTb parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("SamplingRate", &fSamplingRate))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find SamplingRate parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("DriftLength", &fDriftLength))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find DriftLength parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("YDivider", &fYDivider))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find YDivider parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("EIonize", &fEIonize))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find EIonize parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("DriftVelocity", &fDriftVelocity))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find DriftVelocity parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("CoefDiffusionLong", &fCoefDiffusionLong))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find CoefDiffusionLong parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("CoefDiffusionTrans", &fCoefDiffusionTrans))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find CoefDiffusionTrans parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("CoefAttachment", &fCoefAttachment))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find CoefAttachment parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("Gain", &fGain))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find Gain parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("TrackingParFile", &fTrackingParFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find TrackingParFile parameter!");
      return kFALSE;
    }
    fTrackingParFileName = GetFile(fTrackingParFile);
    if (!(paramList -> fill("UAMapFile", &fUAMapFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find UAMapFile parameter!");
      return kFALSE;
    }
    fUAMapFileName = GetFile(fUAMapFile);
    if (!(paramList -> fill("AGETMapFile", &fAGETMapFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find AGETMapFile parameter!");
      return kFALSE;
    }
    fAGETMapFileName = GetFile(fAGETMapFile);
    if (!(paramList -> fill("GainCalibrationDataFile", &fGainCalibrationDataFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find GainCalibrationDataFile parameter!");
      return kFALSE;
    }
    fGainCalibrationDataFileName = GetFile(fGainCalibrationDataFile);
    if (!(paramList -> fill("GCConstant", &fGCConstant))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find GCConstant parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("GCLinear", &fGCLinear))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find GCLinear parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("GCQuadratic", &fGCQuadratic))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find GCQuadratic parameter!");
      return kFALSE;
    }

    if (!(paramList -> fill("YDriftOffset", &fYDriftOffset))) {
      fLogger -> Info(MESSAGE_ORIGIN, "YDriftOffset not found. It will be set to zero.");
      fYDriftOffset = 0;
    }
    if (!(paramList -> fill("TotalADCWhenSat", &fTotalADCWhenSat))) {
      fLogger -> Info(MESSAGE_ORIGIN, "TotalADCWhenSat not found. It will be set to -1 (Neg. values disable this saturation mode)");
      fTotalADCWhenSat = -1;
    }

    fInitialized = kTRUE;
  }

  return kTRUE;
}

Int_t STDigiPar::GetTBTime() {
  switch (fSamplingRate) {
    case 25:
      return 40;
    case 50:
      return 20;
    case 100:
      return 10;
    default:
      return -1;
  }
}


void 
STDigiPar::putParams(FairParamList *paramList)
{
  if (!paramList) {
    fLogger -> Fatal(MESSAGE_ORIGIN, "Parameter list doesn't exist!");
    return;
  }
  paramList -> add("PadPlaneX", fPadPlaneX);
  paramList -> add("PadPlaneZ", fPadPlaneZ);
  paramList -> add("PadSizeX", fPadSizeX);
  paramList -> add("PadSizeZ", fPadSizeZ);
  paramList -> add("PadRows", fPadRows);
  paramList -> add("PadLayers", fPadLayers);
  paramList -> add("AnodeWirePlaneY", fAnodeWirePlaneY);
  paramList -> add("GroundWirePlaneY", fGroundWirePlaneY);
  paramList -> add("GatingWirePlaneY", fGatingWirePlaneY);
  paramList -> add("EField", fEField);
  paramList -> add("FPNPedestalRMS", fFPNPedestalRMS);
  paramList -> add("NumTbs", fNumTbs);
  paramList -> add("WindowNumTbs", fWindowNumTbs);
  paramList -> add("WindowStartTb", fWindowStartTb);
  paramList -> add("SamplingRate", fSamplingRate);
  paramList -> add("DriftLength", fDriftLength);
  paramList -> add("YDivider", fYDivider);
  paramList -> add("EIonize", fEIonize);
  paramList -> add("DriftVelocity", fDriftVelocity);
  paramList -> add("CoefDiffusionLong", fCoefDiffusionLong);
  paramList -> add("CoefDiffusionTrans", fCoefDiffusionTrans);
  paramList -> add("CoefAttachment", fCoefAttachment);
  paramList -> add("Gain", fGain);
  paramList -> add("TrackingParFile", fTrackingParFile);
  paramList -> add("UAMapFile", fUAMapFile);
  paramList -> add("AGETMapFile", fAGETMapFile);
  paramList -> add("GainCalibrationDataFile", fGainCalibrationDataFile);
  paramList -> add("GCConstant", fGCConstant);
  paramList -> add("GCLinear", fGCLinear);
  paramList -> add("GCQuadratic", fGCQuadratic);
  paramList -> add("YDriftOffset", fYDriftOffset);
}

TString 
STDigiPar::GetFile(Int_t fileNum)
{
  std::ifstream fileList;
  TString sysFile = gSystem -> Getenv("VMCWORKDIR");
  TString parFile = sysFile + "/parameters/ST.files.par";
  fileList.open(parFile.Data());

  if(!fileList) { 
    fLogger -> Fatal(MESSAGE_ORIGIN, Form("File %s not found!", parFile.Data()));
    throw;
  }

  Char_t buffer[256];
  for(Int_t iFileNum = 0; iFileNum < fileNum + 1; ++iFileNum){
    if(fileList.eof()) {
      fLogger -> Fatal(MESSAGE_ORIGIN, Form("Did not find string #%d in file %s.", fileNum, parFile.Data()));
      throw;
    }
    fileList.getline(buffer, 256);
  }
  fileList.close();

  return TString(sysFile + "/" + buffer);
}
