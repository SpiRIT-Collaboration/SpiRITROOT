#ifndef RINGSTATECHANGEITEM
#define RINGSTATECHANGEITEM

#include "RingItemBodyHeader.hh"

#include "TString.h"

class RingStateChangeItem : public RingItemBodyHeader {
  public:
    RingStateChangeItem();

       UInt_t GetRunNumber();                       
       UInt_t GetTimeOffset();
       UInt_t GetUnixTimestamp();
       UInt_t GetOffsetDivisor();
       UInt_t GetOriginalSourceID();
      TString GetTitle();

    void Clear(Option_t * = "");
    void Read(ifstream &file, Bool_t rewind = kFALSE);

    void Print();

  private:
    uint32_t fRunNumber;
    uint32_t fTimeOffset;
    uint32_t fUnixTimestamp;
    uint32_t fOffsetDivisor;
    uint32_t fOriginalSourceID;
        char fTitle[81];

  ClassDef(RingStateChangeItem, 1)
};

#endif
