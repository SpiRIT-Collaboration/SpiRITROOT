#include "RingStateChangeItem.hh"

ClassImp(RingStateChangeItem)

RingStateChangeItem::RingStateChangeItem() {
  Clear();
}

 UInt_t RingStateChangeItem::GetRunNumber()        { return fRunNumber; }
 UInt_t RingStateChangeItem::GetTimeOffset()       { return fTimeOffset; }
 UInt_t RingStateChangeItem::GetUnixTimestamp()    { return fUnixTimestamp; }
 UInt_t RingStateChangeItem::GetOffsetDivisor()    { return fOffsetDivisor; }
 UInt_t RingStateChangeItem::GetOriginalSourceID() { return fOffsetDivisor; }
TString RingStateChangeItem::GetTitle()            { return fTitle; }

void RingStateChangeItem::Clear(Option_t *) {
  memset(&fRunNumber,        0, sizeof(uint32_t));
  memset(&fTimeOffset,       0, sizeof(uint32_t));
  memset(&fUnixTimestamp,    0, sizeof(uint32_t));
  memset(&fOffsetDivisor,    0, sizeof(uint32_t));
  memset(&fOriginalSourceID, 0, sizeof(uint32_t));
  memset(fTitle,             0, sizeof(fTitle));
}

void RingStateChangeItem::Read(ifstream &stream, Bool_t rewind) {
  Clear();

  RingItemBodyHeader::Read(stream);

  stream.read((Char_t *) &fRunNumber,         4);
  stream.read((Char_t *) &fTimeOffset,        4);
  stream.read((Char_t *) &fUnixTimestamp,     4);
  stream.read((Char_t *) &fOffsetDivisor,     4);
  stream.read((Char_t *) &fOriginalSourceID,  4);
  stream.read((Char_t *) fTitle,             81);

  stream.seekg((ULong64_t) stream.tellg() - (RINGITEMHEADERSIZE + RingItemBodyHeader::GetSize() + 97)*rewind);
}

void RingStateChangeItem::Print() {
  RingItemBodyHeader::Print();

  cout << " == RingStateChangeItem ===================================================" << endl;
  cout << "         run number: " << GetRunNumber() << endl;
  cout << "        time offset: " << GetTimeOffset() << endl;
  cout << showbase << hex;
  cout << "    unix timesetamp: " << GetUnixTimestamp() << endl;
  cout << noshowbase << hex;
  cout << "     offset divisor: " << GetOffsetDivisor() << endl;
  cout << " original source id: " << GetOriginalSourceID() << endl;
  cout << "              title: " << GetTitle() << endl;
  cout << " ==========================================================================" << endl;
}
