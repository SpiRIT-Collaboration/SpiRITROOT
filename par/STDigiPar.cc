//---------------------------------------------------------------------
// Description:
//      STDigiPar reads in parameters and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//      JungWoo Lee     Korea Univ.
//----------------------------------------------------------------------

#include "STDigiPar.hh"

#include <iostream>

ClassImp(STDigiPar)

STDigiPar::STDigiPar(const Char_t *name, const Char_t *title, const Char_t *context)
:FairParGenericSet("STDigiPar", "SPiRIT Parameter Container", ""),
 fGas(NULL)
{
  fInitialized = kFALSE;

  fLogger = FairLogger::GetLogger();
}

STDigiPar::~STDigiPar()
{
}

// Getters
Double_t  STDigiPar::GetPadPlaneX()        { return fPadPlaneX; }
Double_t  STDigiPar::GetPadPlaneZ()        { return fPadPlaneZ; }
Double_t  STDigiPar::GetPadSizeX()         { return fPadSizeX; }
Double_t  STDigiPar::GetPadSizeZ()         { return fPadSizeZ; }
   Int_t  STDigiPar::GetPadRows()          { return fPadRows; }
   Int_t  STDigiPar::GetPadLayers()        { return fPadLayers; }
Double_t  STDigiPar::GetAnodeWirePlaneY()  { return fAnodeWirePlaneY; }
Double_t  STDigiPar::GetGroundWirePlaneY() { return fGroundWirePlaneY; }
Double_t  STDigiPar::GetGatingWirePlaneY() { return fGatingWirePlaneY; }
   Int_t  STDigiPar::GetNumTbs()           { return fNumTbs; }
   Int_t  STDigiPar::GetWindowNumTbs()     { return fWindowNumTbs; }
   Int_t  STDigiPar::GetWindowStartTb()    { return fWindowStartTb; }
Double_t  STDigiPar::GetDriftVelocity()    { return fDriftVelocity; }
Double_t  STDigiPar::GetDriftLength()      { return fDriftLength; }
   Int_t  STDigiPar::GetYDivider()         { return fYDivider; }

STGas *STDigiPar::GetGas()
{ 
  if(fGas==NULL){
    std::cerr << "Initializing gas file with " << fGasFileName.Data() << std::endl;
    fGas = new STGas(fGasFileName.Data());
  }
  return fGas;
  //return new STGas(*fGas); 
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

Bool_t STDigiPar::getParams(FairParamList *paramList)
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
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadRows parameter!");
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
    if (!(paramList -> fill("DriftVelocity", &fDriftVelocity))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find DriftVelocity parameter!");
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
    if (!(paramList -> fill("GasFile", &fGasFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find GasFile parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadPlaneFile", &fPadPlaneFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadPlaneFile parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadShapeFile", &fPadShapeFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadShapeFile parameter!");
      return kFALSE;
    }

    fGasFileName = GetFile(fGasFile);
  }

  return kTRUE;
}

void STDigiPar::putParams(FairParamList *paramList)
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
  paramList -> add("NumTbs", fNumTbs);
  paramList -> add("WindowNumTbs", fWindowNumTbs);
  paramList -> add("WindowStartTb", fWindowStartTb);
  paramList -> add("SamplingRate", fSamplingRate);
  paramList -> add("DriftVelocity", fDriftVelocity);
  paramList -> add("DriftLength", fDriftLength);
  paramList -> add("YDivider", fYDivider);
  paramList -> add("GasFile", fGasFile);
  paramList -> add("PadPlaneFile", fPadPlaneFile);
  paramList -> add("PadShapeFile", fPadShapeFile);
}

TString STDigiPar::GetFile(Int_t fileNum)
{
  std::ifstream fileList;
  TString sysFile = gSystem -> Getenv("VMCWORKDIR");
  TString parFile = sysFile + "/parameters/ST.files.par";
  fileList.open(parFile.Data());

  if(!fileList) { fLogger -> Fatal(MESSAGE_ORIGIN, Form("File %s not found!", parFile.Data()));

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
