#ifndef RINGITEMHEADER
#define RINGITEMHEADER

#define RINGITEMHEADERSIZE    8

// We only expect three RingItem types in the data
#define RINGITEMBEGINRUN      1
#define RINGITEMENDRUN        2
#define RINGITEMPHYSICSEVENT 30

#include "TObject.h"

#include <fstream>
#include <iostream>

using namespace std;

class RingItemHeader : public TObject {
  public:
    RingItemHeader();

       UInt_t GetSize();
       UInt_t GetType();

    void Clear(Option_t * = "");
    void Read(ifstream &file, Bool_t rewind = kFALSE);

    void Print();

  private:
    uint32_t fSize;
    uint32_t fType;

  ClassDef(RingItemHeader, 1)
};

#endif
