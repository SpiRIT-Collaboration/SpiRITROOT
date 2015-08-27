// =================================================
//  STMapTest Class
// 
//  Description:
//    Test AGET & UnitAsAd map with plot
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 30
// =================================================

#ifndef STMAPTEST_H
#define STMAPTEST_H

#include "TObject.h"
#include "TString.h"

#include "TCanvas.h"
#include "TH2D.h"
#include "TList.h"

#include "STMap.hh"

class STMapTest : public TObject
{
  public:
    STMapTest();
    ~STMapTest();

    void SetUAMap(TString filename);
    void SetAGETMap(TString filename);

    void ShowAGETMap(Int_t UAIdx = 0);
    void ShowUAMap();
    void PrintMap(Int_t padRow, Int_t padLayer);
    void PrintMap(Int_t coboIdx, Int_t asadIdx, Int_t agetIdx, Int_t chIdx);

  private:
    STMap *map;

    TCanvas *agetCvs;
    TH2D *agetHist;

    TCanvas *uaCvs;
    TH2D *uaHist;
    TList *uaList;
    TList *uaMapList;
    TList *uaLineList;

    Bool_t fIsUAMap;
    Bool_t fIsAGETMap;

  ClassDef(STMapTest, 1);
};

#endif
