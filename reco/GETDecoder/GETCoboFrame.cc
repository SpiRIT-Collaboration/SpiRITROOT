#include "GETCoboFrame.hh"

GETCoboFrame::GETCoboFrame() {
  Clear();
}

void GETCoboFrame::ReadFrame(ifstream &stream) {
  fFrame[fNumFrames++].Read(stream);
}

void GETCoboFrame::ReadFrame(Int_t index, ifstream &stream) {
  fFrame[index].Clear();
  fFrame[index].Read(stream);
}

        Int_t  GETCoboFrame::GetNumFrames()        { return fNumFrames; }
GETBasicFrame *GETCoboFrame::GetFrames()           { return fFrame; }
GETBasicFrame *GETCoboFrame::GetFrame(Int_t index) { return &fFrame[index]; }

void GETCoboFrame::Clear(Option_t *) {
  for (Int_t iFrame = 0; iFrame < fNumFrames; iFrame++)
    fFrame[iFrame].Clear();

  fNumFrames = 0;
}
