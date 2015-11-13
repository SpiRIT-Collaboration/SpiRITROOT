#ifndef GETLAYERHEADER
#define GETLAYERHEADER

#include "GETHeaderBase.hh"

#define GETLAYERHEADERBYIDSIZE   (GETHEADERBASESIZE + 12)
#define GETLAYERHEADERBYTIMESIZE (GETHEADERBASESIZE + 16)

class GETLayerHeader : public GETHeaderBase {
  public:
    GETLayerHeader();

       UInt_t GetHeaderSize(Bool_t inBytes = kTRUE);
       UInt_t GetItemSize();
       UInt_t GetNItems();
       UInt_t GetEventID();
    ULong64_t GetEventTime();
       UInt_t GetDeltaT();
    ULong64_t GetFrameSkip();
       UInt_t GetHeaderSkip();

    void Clear(Option_t * = "");
    void Read(ifstream &stream);

    void Print();

  private:
    uint8_t fHeaderSize[2];
    uint8_t fItemSize[2];
    uint8_t fNItems[4];
    uint8_t fEventID[4];
    uint8_t fEventTime[6];
    uint8_t fDeltaT[2];

  ClassDef(GETLayerHeader, 1)
};

#endif
