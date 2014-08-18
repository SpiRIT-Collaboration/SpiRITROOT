//---------------------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      STGas reads in gas property file and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STGas.hh"

// ROOT class headers
#include "TRandom.h"

ClassImp(STGas)

STGas::STGas()
{
  fInitialized = kFALSE;

  fLogger = FairLogger::GetLogger();
}

STGas::~STGas()
{
}

void STGas::operator=(const STGas& GasToCopy) { fEIonize = GasToCopy.fEIonize; }

Double_t STGas::GetEIonize()            { return fEIonize; }
Double_t STGas::GetDriftVelocity()      { return fDriftVelocity; }
Double_t STGas::GetCoefAttachment()     { return fCoefAttachment; }
Double_t STGas::GetCoefDiffusionLong()  { return fCoefDiffusionLong; }
Double_t STGas::GetCoefDiffusionTrans() { return fCoefDiffusionTrans; }
Int_t    STGas::GetGain()               { return fGain; }
UInt_t   STGas::GetRandomCS()           {
  UInt_t CS = (UInt_t)(gRandom -> Gaus(50,20));
  if(CS==0) CS=1;
  return CS;
}

Bool_t STGas::getParams(FairParamList *paramList)
{
  if(!paramList){
    fLogger -> Fatal(MESSAGE_ORIGIN, "Parameter list doesn't exist!");
    return kFALSE;
  }

  if (!fInitialized){
    if(!(paramList -> fill("EIonizeP10", &fEIonize))){
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find EIonize parameter!");
      return kFALSE;
    }
    if(!(paramList -> fill("DriftVelocity", &fDriftVelocity))){
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find DriftVelocity parameter!");
      return kFALSE;
    }
    if(!(paramList -> fill("CoefAttachment", &fCoefAttachment))){
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find CoefAttachment parameter!");
      return kFALSE;
    }
    if(!(paramList -> fill("CoefDiffusionLong", &fCoefDiffusionLong))){
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find CoefDiffusionLong parameter!");
      return kFALSE;
    }
    if(!(paramList -> fill("CoefDiffusionTrans", &fCoefDiffusionTrans))){
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find CoefDiffusionTrans parameter!");
      return kFALSE;
    }
    if(!(paramList -> fill("Gain", &fGain))){
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find Gain parameter!");
      return kFALSE;
    }
  }

}

void STGas::putParams(FairParamList *paramList)
{
  if(!paramList){
    fLogger -> Fatal(MESSAGE_ORIGIN,"Parameter list doesn't exist!");
    return;
  }
  paramList -> add("EIonize", fEIonize);
  paramList -> add("DriftVelocity", fDriftVelocity);
  paramList -> add("CoefAttachment", fCoefAttachment);
  paramList -> add("CoefDiffusionLong", fCoefDiffusionLong);
  paramList -> add("CoefDiffusionTrans", fCoefDiffusionTrans);
  paramList -> add("Gain", fGain);
}

TString STGas::GetFile(Int_t fileNum)
{
  ifstream fileList;
  TString sysFile = gSystem -> Getenv("SPIRITDIR");
  TString parFile = sysFile + "/parameters/ST.gas.par";
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
