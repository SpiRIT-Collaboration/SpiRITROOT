// =================================================
//  STPadRemover Class
// 
//  Description:
//    Removes pads from being analyzed
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2017. 02. 06
// =================================================

#ifndef STPADREMOVER
#define STPADREMOVER

#include <vector>
#include <utility>
#include <iostream>
#include <fstream>

#include "TString.h"

class STPadRemoverData {
  public:
    Int_t fEventID;
    Bool_t fPads[12096];
};

class STPadRemover {
  public:
    STPadRemover();
    STPadRemover(TString datafile);
    ~STPadRemover() {};

    Bool_t Init();
    void SetDatafile(TString datafile);
    Bool_t CheckBadPad(Int_t row, Int_t layer, Int_t eventid = 0);

  private:
    void ClassInit();
    Bool_t FindEvent(Int_t eventid = 0);

    Bool_t fIsInitialized;
    TString fDatafile;

//    Int_t fEventID;
    STPadRemoverData *fEvent;
    std::vector<STPadRemoverData> fData;
//    Bool_t fPads[12096];
//    std::vector<std::pair<Int_t, Int_t>> fEventList;
//    std::vector<std::pair<Int_t, Int_t>> fPadList;

  ClassDef(STPadRemover, 1)
};

#endif
