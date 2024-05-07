#include "RingItemHeader.hh"

ClassImp(RingItemHeader)

RingItemHeader::RingItemHeader() {
  Clear();
}

UInt_t RingItemHeader::GetSize() { return fSize; }
UInt_t RingItemHeader::GetType() { return fType; }

void RingItemHeader::Clear(Option_t *) {
  memset(&fSize, 0, sizeof(uint32_t));
  memset(&fType, 0, sizeof(uint32_t));
}

void RingItemHeader::Read(ifstream &stream, Bool_t rewind) {
  Clear();

  stream.read((Char_t *) &fSize, 4);
  stream.read((Char_t *) &fType, 4);

  stream.seekg((ULong64_t) stream.tellg() - RINGITEMHEADERSIZE*rewind);
}

void RingItemHeader::Print() {
  cout << " == RingItemHeader ==" << endl;
  cout << "    size: " << GetSize() << endl;
  cout << "    type: " << GetType() << endl;
  cout << " ====================" << endl;
}
