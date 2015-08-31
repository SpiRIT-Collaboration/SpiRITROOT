// =================================================
//  STParReader Class
// 
//  Description:
//    Read parameter file written for SpiRITROOT
//    and returns the parameters
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2015. 09. 01
// =================================================

// GETDecoder
#include "GETFileChecker.hh"

// STConverter
#include "STParReader.hh"

// STL
#include <iostream>
#include <fstream>

ClassImp(STParReader)

STParReader::STParReader()
{
  Init();
}

STParReader::STParReader(TString parFile)
{
  Init();

  fIsGood = SetParFile(parFile);
}

void
STParReader::Init()
{
  fParFile = "";
  fIsGood = kFALSE;
}

Bool_t
STParReader::SetParFile(TString parFile)
{
  fParFile = GETFileChecker::CheckFile(parFile);

  if (!fParFile.EqualTo(""))
    fIsGood = kTRUE;

  return fIsGood;
}

Bool_t
STParReader::IsGood()
{
  return fIsGood;
}

Int_t
STParReader::GetIntPar(TString parName)
{
  std::ifstream parameters(fParFile.Data());
  while (kTRUE) {
    TString value;
    value.ReadToken(parameters);
    if (value.EqualTo(Form("%s:Int_t", parName.Data()))) {
      value.ReadToken(parameters);
      parameters.close();
      return value.Atoi();
    }

    if (parameters.eof()) {
      std::cout << "== [STParReader] Integer parameter '" << parName.Data() << "' not found!" << std::endl;

      parameters.close();
      return -1;
    }
  }
}

Double_t
STParReader::GetDoublePar(TString parName)
{
  std::ifstream parameters(fParFile.Data());
  while (kTRUE) {
    TString value;
    value.ReadToken(parameters);
    if (value.EqualTo(Form("%s:Double_t", parName.Data()))) {
      value.ReadToken(parameters);
      parameters.close();
      return value.Atof();
    }

    if (parameters.eof()) {
      std::cout << "== [STParReader] Double parameter '" << parName.Data() << "' not found!" << std::endl;

      parameters.close();
      return -1;
    }
  }
}

TString
STParReader::GetFilePar(Int_t index)
{
  TString listFile = fParFile;
  listFile.ReplaceAll("ST.parameters.par", "ST.files.par");

  std::ifstream fileList(listFile.Data());

  Char_t buffer[256];
  for (Int_t iFileNum = 0; iFileNum < index + 1; iFileNum++) {
    if (fileList.eof()) {
      std::cout << "== [STParReader] Cannot find file #" << iFileNum << std::endl;

      fileList.close();
      return "";
    }

    fileList.getline(buffer, 256);
  }

  fileList.close();

  return listFile.ReplaceAll("parameters/ST.files.par", buffer);
}
