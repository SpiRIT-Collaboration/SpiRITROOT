#ifndef GETBASICFRAMEHEADER
#define GETBASICFRAMEHEADER

#include "GETHeaderBase.hh"

#define GETBASICFRAMEHEADERSIZE (GETHEADERBASESIZE + 79)

#include <bitset>

using std::bitset;

class GETBasicFrameHeader : public GETHeaderBase {
  public:
    GETBasicFrameHeader();

        UInt_t GetHeaderSize(Bool_t inBytes = kTRUE);
        UInt_t GetItemSize();
        UInt_t GetNItems();
       ULong_t GetEventTime();
        UInt_t GetEventID();
        UInt_t GetCoboID();
        UInt_t GetAsadID();
        UInt_t GetReadOffset();
        UInt_t GetStatus();
    bitset<72> GetHitPat(Int_t asadID);
        UInt_t GetMultip(Int_t asadID);
        UInt_t GetWindowOut();
        UInt_t GetLastCell(Int_t asadID);
     ULong64_t GetFrameSkip();
        UInt_t GetHeaderSkip();

    void Clear(Option_t * = "");
    void Read(ifstream &stream);

    void Print();

  private:
    uint8_t fHeaderSize[2];
    uint8_t fItemSize[2];
    uint8_t fNItems[4];
    uint8_t fEventTime[6];
    uint8_t fEventID[4];
    uint8_t fCoboID;
    uint8_t fAsadID;
    uint8_t fReadOffset[2];
    uint8_t fStatus;
    uint8_t fHitPat[4][9];
    uint8_t fMultip[4][2];
    uint8_t fWindowOut[4];
    uint8_t fLastCell[4][2];

  ClassDef(GETBasicFrameHeader, 1)
};

#endif
