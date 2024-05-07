#ifndef RINGPHYSICSEVENTITEM
#define RINGPHYSICSEVENTITEM

#include "RingItemBodyHeader.hh"

#include "GETBasicFrame.hh"

class RingPhysicsEventItem : public RingItemBodyHeader {
  public:
    RingPhysicsEventItem();

    GETBasicFrame *GetGETBasicFrame();

    void Clear(Option_t * = "");
    void Read(ifstream &file, Bool_t rewind = kFALSE);

    void Print();

  private:
    GETBasicFrame fGETBasicFrame;

  ClassDef(RingPhysicsEventItem, 1)
};

#endif
