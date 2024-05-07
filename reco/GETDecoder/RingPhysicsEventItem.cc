#include "RingPhysicsEventItem.hh"
#include "RingItemBodyHeader.hh"

ClassImp(RingPhysicsEventItem)

RingPhysicsEventItem::RingPhysicsEventItem() {
  Clear();
}

GETBasicFrame *RingPhysicsEventItem::GetGETBasicFrame() { return &fGETBasicFrame; }

void RingPhysicsEventItem::Clear(Option_t *) {
}

void RingPhysicsEventItem::Read(ifstream &stream, Bool_t rewind) {
  Clear();

  RingItemBodyHeader::Read(stream);

  fGETBasicFrame.Read(stream);
}

void RingPhysicsEventItem::Print() {
  RingItemBodyHeader::Print();

  fGETBasicFrame.Print();
}
