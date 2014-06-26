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

class STRawEvent : public TNamed {
  public:
    STRawEvent();
    ~STRawEvent();

    void PrintPads();

    // setters
    void SetEventID(Int_t evtid);
    void SetPad(STPad *pad);

    // getters
    Int_t GetEventID();
    Int_t GetNumPads();
    STPad *GetPad(Int_t padNo);
    STPad *GetPad(Int_t row, Int_t layer); 

  private:
    Int_t fEventID;
    Int_t fNumPads;
    STPad *fPadsArray[12096];

  ClassDef(STRawEvent, 1);
};

#endif
