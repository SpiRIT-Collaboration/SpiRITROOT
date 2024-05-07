#include "RingItemBodyHeader.hh"

ClassImp(RingItemBodyHeader)

RingItemBodyHeader::RingItemBodyHeader() {
  Clear();
}

 UInt_t RingItemBodyHeader::GetSize()        { return fSize; }
ULong_t RingItemBodyHeader::GetTimestamp()   { return fTimestamp; }
 UInt_t RingItemBodyHeader::GetSourceID()    { return fSourceID; }
 UInt_t RingItemBodyHeader::GetBarrierType() { return fBarrierType; }

void RingItemBodyHeader::Clear(Option_t *) {
  memset(&fSize,        0, sizeof(uint32_t));
  memset(&fTimestamp,   0, sizeof(uint64_t));
  memset(&fSourceID,    0, sizeof(uint32_t));
  memset(&fBarrierType, 0, sizeof(uint32_t));
}

void RingItemBodyHeader::Read(ifstream &stream, Bool_t rewind) {
  Clear();

  RingItemHeader::Read(stream);

  stream.read((Char_t *) &fSize, 4);

  if (fSize > 4) {
    stream.read((Char_t *) &fTimestamp,   8);
    stream.read((Char_t *) &fSourceID,    4);
    stream.read((Char_t *) &fBarrierType, 4);
  }

  stream.seekg((ULong64_t) stream.tellg() - (RINGITEMHEADERSIZE + fSize)*rewind);
}

void RingItemBodyHeader::Print() {
  RingItemHeader::Print();

  cout << " == RingItemBodyHeader ===============" << endl;
  cout << "            size: " << GetSize() << endl;
  cout << showbase << hex;
  cout << "       timestamp: " << GetTimestamp() << endl;
  cout << noshowbase << hex;
  cout << "       source id: " << GetSourceID() << endl;
  cout << "    barrier type: " << GetBarrierType() << endl;
  cout << " =====================================" << endl;
}
