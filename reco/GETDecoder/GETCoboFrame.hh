#ifndef GETCOBOFRAME
#define GETCOBOFRAME

#include "GETBasicFrame.hh"

class GETCoboFrame {
  public:
    GETCoboFrame();

             void  ReadFrame(ifstream &stream);
             void  ReadFrame(Int_t index, ifstream &stream);

            Int_t  GetEventID();
            Int_t  GetNumFrames();
    GETBasicFrame *GetFrames();
    GETBasicFrame *GetFrame(Int_t index);

             void  Clear(Option_t * = "");

  private:
            Int_t  fNumFrames;
    GETBasicFrame  fFrame[4];

  ClassDef(GETCoboFrame, 1);
};

#endif
