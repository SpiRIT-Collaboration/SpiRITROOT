// =================================================
//  STRawEvent Class                          
//                                                  
//  Description:                                    
//    Container for a raw event data
//                                                  
//  Genie Jhang ( geniejhang@majimak.com )          
//  2013. 08. 16                                    
// =================================================

#ifndef STRAWEVENT_H
#define STRAWEVENT_H

#include "TROOT.h"
#include "TObject.h"

#include "STPad.hh"

#include <vector>

class STRawEvent : public TNamed {
  public:
    STRawEvent();
    STRawEvent(STRawEvent *object);
    ~STRawEvent();

    void PrintPads();
    void Clear();

    // setters
    void SetEventID(Int_t evtid);
    void SetPad(STPad *pad);
    void SetIsGood(Bool_t value);
    void RemovePad(Int_t padNo);
    void RemovePad(Int_t row, Int_t layer);

    // getters
    Int_t GetEventID();
    Int_t GetNumPads();
    Bool_t IsGood();

    std::vector<STPad> *GetPads();

    STPad *GetPad(Int_t padNo);
    STPad *GetPad(Int_t row, Int_t layer); 

  private:
    Int_t fEventID;
    std::vector<STPad> fPadArray;

    Bool_t fIsGood;

  ClassDef(STRawEvent, 3);
};

#endif
