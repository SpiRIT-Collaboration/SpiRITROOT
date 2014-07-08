//---------------------------------------------------------------------
// Description:
//      STDigiPar reads in parameters and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//----------------------------------------------------------------------

#include "STDigiPar.hh"

ClassImp(STDigiPar)

STDigiPar::STDigiPar(const Char_t *name, const Char_t *title, const Char_t *context)
:FairParGenericSet("STDigiPar", "SPiRIT Parameter Container", "")
{
  fInitialized = kFALSE;

  fLogger = FairLogger::GetLogger();
}

STDigiPar::~STDigiPar()
{
}

// Getters
Int_t STDigiPar::GetPadPlaneX()
{
  return fPadPlaneX;
}

Int_t STDigiPar::GetPadPlaneZ()
{
  return fPadPlaneZ;
}

Int_t STDigiPar::GetPadSizeX()
{
  return fPadSizeX;
}

Int_t STDigiPar::GetPadSizeZ()
{
  return fPadSizeZ;
}

Double_t STDigiPar::GetWirePlaneY()
{
  return fWirePlaneY;
}

STGas *STDigiPar::GetGas()
{
  return fGas;
}

Int_t STDigiPar::GetNumTbs()
{
  return fNumTbs;
}

Int_t STDigiPar::GetTBTime()
{
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

Double_t STDigiPar::GetDriftVelocity()
{
  return fDriftVelocity;
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
    if (!(paramList -> fill("WirePlaneY", &fWirePlaneY))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find WirePlaneY parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("GasFile", &fGasFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find GasFile parameter!");
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
    if (!(paramList -> fill("SamplingRate", &fSamplingRate))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find SamplingRate parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("DriftVelocity", &fDriftVelocity))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find DriftVelocity parameter!");
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
  paramList -> add("WirePlaneY", fWirePlaneY);
  paramList -> add("GasFile", fGasFile);
  paramList -> add("EField", fEField);
  paramList -> add("NumTbs", fNumTbs);
  paramList -> add("SamplingRate", fSamplingRate);
  paramList -> add("DriftVelocity", fDriftVelocity);
  paramList -> add("PadPlaneFile", fPadPlaneFile);
  paramList -> add("PadShapeFile", fPadShapeFile);
}

TString STDigiPar::GetFile(Int_t fileNum)
{
  ifstream fileList;
  TString sysFile = gSystem -> Getenv("SPIRITDIR");
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
