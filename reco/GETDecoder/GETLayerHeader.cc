#include "GETLayerHeader.hh"

ClassImp(GETLayerHeader)

GETLayerHeader::GETLayerHeader() {
  Clear();
}

   UInt_t GETLayerHeader::GetHeaderSize(Bool_t inBytes)  { return CorrectEndianness(fHeaderSize, 2)*(inBytes ? GetUnitBlock() : 1); }
   UInt_t GETLayerHeader::GetItemSize()                  { return CorrectEndianness(fItemSize, 2); }
   UInt_t GETLayerHeader::GetNItems()                    { return CorrectEndianness(fNItems, 4); }
   UInt_t GETLayerHeader::GetEventID()                   { return CorrectEndianness(fEventID, 4); }
ULong64_t GETLayerHeader::GetEventTime()                 { return CorrectEndianness(fEventTime, 6); }
   UInt_t GETLayerHeader::GetDeltaT()                    { return CorrectEndianness(fDeltaT, 2); }
ULong64_t GETLayerHeader::GetFrameSkip()                 { return GetFrameSize() - GetHeaderSize(); }

UInt_t GETLayerHeader::GetHeaderSkip() {
  switch (GetFrameType()) {
    case GETFRAMEMERGEDBYID:
      return GetHeaderSize() - GETLAYERHEADERBYIDSIZE;

    case GETFRAMEMERGEDBYTIME:
      return GetHeaderSize() - GETLAYERHEADERBYTIMESIZE;
  }

  return 0;
}
 
void GETLayerHeader::Clear(Option_t *) {
  GETHeaderBase::Clear();

  memset(fHeaderSize, 0, sizeof(uint8_t)*2);
  memset(fItemSize,   0, sizeof(uint8_t)*2);
  memset(fNItems,     0, sizeof(uint8_t)*4);
  memset(fEventID,    0, sizeof(uint8_t)*4);
  memset(fEventTime,  0, sizeof(uint8_t)*6);
  memset(fDeltaT,     0, sizeof(uint8_t)*2);
}

void GETLayerHeader::Read(ifstream &stream) {
  Clear();

  GETHeaderBase::Read(stream);

  stream.read((Char_t *) fHeaderSize, 2);
  stream.read((Char_t *)   fItemSize, 2);
  stream.read((Char_t *)     fNItems, 4);
  switch (GetFrameType()) {
    case GETFRAMEMERGEDBYID:
      stream.read((Char_t *)   fEventID,   4);
      break;

    case GETFRAMEMERGEDBYTIME:
      stream.read((Char_t *)   fEventTime, 6);
      stream.read((Char_t *)   fDeltaT,    2);
      break;
  }

  stream.ignore(GetHeaderSkip());
}

void GETLayerHeader::Print() {
  cout << showbase << hex;
  cout << " == GETLayerHeader ======================" << endl;
  cout << "    metaType: " << GetMetaType() << endl;
  cout << "              - Endianness: " << (IsLittleEndian() ? "Little" : "Big") << endl;
  cout << "              -   Blobness: " << (IsBlob() ? "YES" : "NO") << endl;
  cout << "              -  UnitBlock: " << dec << GetUnitBlock() << hex << endl;
  cout << "   frameSize: " << GetFrameSize(kFALSE) << " (" << dec << GetFrameSize(kFALSE) << " Blocks = " << GetFrameSize() << hex << " Bytes)" << endl;
  cout << "  dataSource: " << GetDataSource() << endl;
  cout << "   frameType: " << GetFrameType() << endl;
  cout << "    revision: " << GetRevision() << endl;
  cout << "  headerSize: " << GetHeaderSize(kFALSE) << " (" << dec << GetHeaderSize(kFALSE) << " Blocks = " << GetHeaderSize() << hex << " Bytes)" << endl;
  cout << "    itemSize: " << GetItemSize() << endl;
  cout << "      nItems: " << GetNItems() << dec << " (" << GetNItems() << ")" << hex << endl;
  switch (GetFrameType()) {
    case GETFRAMEMERGEDBYID:
      cout << "     eventID: " << dec << GetEventID() << endl;
      break;

    case GETFRAMEMERGEDBYTIME:
      cout << "   eventTime: " << GetEventTime() << dec << " (" << GetEventTime() << ")" << hex << endl;
      cout << "      deltaT: " << GetDeltaT() << dec << " (" << GetDeltaT() << ")" << endl;
      break;
  }
  cout << " ========================================" << endl;
}
