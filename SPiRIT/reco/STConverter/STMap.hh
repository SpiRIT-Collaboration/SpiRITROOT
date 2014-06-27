// =================================================
//  STMap Class
// 
//  Description:
//    This class is used for finding the local pad
//    coordinates corresponding to user-input
//    agetIdx & chIdx using map by Tadaaki Isobe.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 13
// =================================================

#ifndef STMAP_H
#define STMAP_H

#include "TROOT.h"
#include "TObject.h"
#include "TString.h"

class STMap : public TObject {
  public:
    STMap();
    ~STMap() {}

    // Setter
    void SetUAMap(TString filename);
    void SetAGETMap(TString filename);

    // Getter
    Bool_t IsSetUAMap();
    Bool_t IsSetAGETMap();

    void GetRowNLayer(Int_t coboIdx, Int_t asadIdx, Int_t agetIdx, Int_t chIdx, Int_t &padRow, Int_t &padLayer);
    void GetMapData(Int_t padRow, Int_t padLayer, Int_t &UAIdx, Int_t &coboIdx, Int_t &asadIdx, Int_t &agetIdx, Int_t &chIdx);

    Int_t GetUAIdx(Int_t coboIdx, Int_t asadIdx);
    Int_t GetCoboIdx(Int_t uaIdx);
    Int_t GetAsadIdx(Int_t uaIdx);

  private:
    Bool_t fIsSetUAMap;
    Bool_t fIsSetAGETMap;

    Int_t padRowOfCh[68];
    Int_t padLayerOfCh[68];

    Int_t ua[48];
    Int_t cobo[48];
    Int_t asad[48];

  ClassDef(STMap, 1);
};

#endif
