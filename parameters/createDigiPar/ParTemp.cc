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

// USER-GETTER

Bool_t 
STDigiPar::getParams(FairParamList *paramList)
{
  if (!paramList) {
    fLogger -> Fatal(MESSAGE_ORIGIN, "Parameter list doesn't exist!");
    return kFALSE;
  }

  if (!fInitialized) {
    // USER-GETPARAMS
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

  // USER-PUTPARAMS
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
