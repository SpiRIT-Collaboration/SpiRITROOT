// =================================================
//  STCore Class
// 
//  Description:
//    Process CoBoFrame data into STRawEvent data
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 09. 25
// =================================================

#ifndef _STCORE_H_
#define _STCORE_H_

#include "TObject.h"

#include "STRawEvent.hh"
#include "STMap.hh"
#include "STPedestal.hh"
#include "GETDecoder.hh"
#include "GETFrame.hh"

class STCore : public TObject {
  public:
    STCore();
    STCore(Char_t *filename);
    STCore(Char_t *filename, Int_t numTbs);
    ~STCore();

    void Initialize();

    // setters
    void AddGraw(Char_t *filename);
    void SetNumTbs(Int_t value);
    void SetInternalPedestal(Int_t startTb = 10, Int_t numTbs = 20);
    void SetPedestalData(Char_t *filename);

    // getters
    STRawEvent *GetRawEvent(Int_t eventID = -1);
    Int_t GetNumTbs();

  private:
    STMap *fMapPtr;

    GETDecoder *fDecoderPtr;
    Bool_t fIsGraw;

    STPedestal *fPedestalPtr;
    Bool_t fIsInternalPedestal;
    Bool_t fIsPedestalData;
    Int_t fStartTb;
    Int_t fNumTbs;

    STRawEvent *fRawEventPtr;

    UInt_t fPrevEventNo;
    UInt_t fCurrEventNo;

    Int_t fCurrFrameNo;

  ClassDef(STCore, 1);
};

#endif
