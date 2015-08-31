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

#ifndef STPARREADER
#define STPARREADER

#include "TObject.h"
#include "TString.h"

class STParReader : public TObject {
  public:
    STParReader();
    STParReader(TString parFile);
    ~STParReader() {};

    Bool_t SetParFile(TString parFile);
    Bool_t IsGood();

    Int_t GetIntPar(TString parName);
    Double_t GetDoublePar(TString parName);
    TString GetFilePar(Int_t index);

  private:
    void Init();

    TString fParFile;
    Bool_t fIsGood;

  ClassDef(STParReader, 1)
};

#endif
