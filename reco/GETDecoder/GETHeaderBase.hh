#ifndef GETHEADERBASE
#define GETHEADERBASE

#define GETHEADERBASESIZE         8

#define GETFRAMETOPOLOGY        0x7
#define GETFRAMEBASICTYPE1      0x1
#define GETFRAMEBASICTYPE2      0x2
#define GETFRAMEMERGEDBYID   0xff01
#define GETFRAMEMERGEDBYTIME 0xff02
#define GETFRAMEMUTANT          0x8

#include "TObject.h"

#include <fstream>
#include <iostream>
#include <bitset>
#include <cmath>
#include <cstdio>

using namespace std;

class GETHeaderBase : public TObject {
  public:
    GETHeaderBase();

       UInt_t GetMetaType();                       
       UInt_t GetFrameSize(Bool_t inBytes = kTRUE);
       UInt_t GetDataSource();
       UInt_t GetFrameType();
       UInt_t GetRevision();
    ULong64_t GetFrameSkip(Bool_t rewind = kFALSE);
  
       Bool_t IsLittleEndian();
       Bool_t IsBlob();
       UInt_t GetUnitBlock();

    ULong64_t CorrectEndianness(uint8_t *variable, Short_t length);

    void Clear(Option_t * = "");
    void Read(ifstream &file, Bool_t rewind = kFALSE);

    void Print();

  private:
    uint8_t fMetaType;
    uint8_t fFrameSize[3];
    uint8_t fDataSource;
    uint8_t fFrameType[2];
    uint8_t fRevision;

  ClassDef(GETHeaderBase, 1)
};

#endif
