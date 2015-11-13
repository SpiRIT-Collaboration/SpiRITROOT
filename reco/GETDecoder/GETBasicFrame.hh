#ifndef GETBASICFRAME
#define GETBASICFRAME

#include "GETBasicFrameHeader.hh"

class GETBasicFrame : public GETBasicFrameHeader {
  public:
    GETBasicFrame();

       Int_t *GetSample(Int_t agetIdx, Int_t chIdx);

       Int_t  GetFrameSkip();

        void  Clear(Option_t * = "");
        void  Read(ifstream &stream);

  private:
       Int_t fSample[4*68*512];

      UInt_t GetIndex(Int_t agetIdx, Int_t chIdx, Int_t tbIdx);

  ClassDef(GETBasicFrame, 1)
};

#endif
