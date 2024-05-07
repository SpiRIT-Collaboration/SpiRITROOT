#ifndef RINGITEMBODYHEADER
#define RINGITEMBODYHEADER

#include "RingItemHeader.hh"

class RingItemBodyHeader : public RingItemHeader {
  public:
    RingItemBodyHeader();

       UInt_t GetSize();
      ULong_t GetTimestamp();
       UInt_t GetSourceID();
       UInt_t GetBarrierType();

    void Clear(Option_t * = "");
    void Read(ifstream &file, Bool_t rewind = kFALSE);

    void Print();

  private:
    uint32_t fSize;
    uint64_t fTimestamp;
    uint32_t fSourceID;
    uint32_t fBarrierType;

  ClassDef(RingItemBodyHeader, 1)
};

#endif
