#include "GETBasicFrameHeader.hh"

ClassImp(GETBasicFrameHeader)

GETBasicFrameHeader::GETBasicFrameHeader() {
  Clear();
}

 UInt_t GETBasicFrameHeader::GetHeaderSize(Bool_t inBytes)  { return CorrectEndianness(fHeaderSize, 2)*(inBytes ? GetUnitBlock() : 1); }
 UInt_t GETBasicFrameHeader::GetItemSize()                  { return CorrectEndianness(fItemSize, 2); }
 UInt_t GETBasicFrameHeader::GetNItems()                    { return CorrectEndianness(fNItems, 4); }
ULong_t GETBasicFrameHeader::GetEventTime()                 { return CorrectEndianness(fEventTime, 6); }
 UInt_t GETBasicFrameHeader::GetEventID()                   { return CorrectEndianness(fEventID, 4); }
 UInt_t GETBasicFrameHeader::GetCoboID()                    { return (UInt_t) fCoboID; }
 UInt_t GETBasicFrameHeader::GetAsadID()                    { return (UInt_t) fAsadID; }
 UInt_t GETBasicFrameHeader::GetReadOffset()                { return CorrectEndianness(fReadOffset, 2); }
 UInt_t GETBasicFrameHeader::GetStatus()                    { return (UInt_t) fStatus; }

bitset<72> GETBasicFrameHeader::GetHitPat(Int_t asadID) {
  bitset<72> tempHitPat;

  if (!IsLittleEndian()) {
    for (Int_t iByte = 0; iByte < 9; iByte++)
      for (Int_t iBit = 0; iBit < 8; iBit++)
        if ((fHitPat[asadID][iByte] >> iBit)&0x1)
          tempHitPat.set((9 - iByte - 1)*8 + iBit);
  } else {
    for (Int_t iByte = 0; iByte < 9; iByte++)
      for (Int_t iBit = 0; iBit < 8; iBit++)
        if ((fHitPat[asadID][iByte] >> iBit)&0x1)
          tempHitPat.set(iByte*8 + iBit);
  }

  return tempHitPat;
}

UInt_t GETBasicFrameHeader::GetMultip(Int_t asadID) {
  uint8_t *tempMultip = fMultip[0] + 2*asadID;
  return CorrectEndianness(tempMultip, 2);
}

UInt_t GETBasicFrameHeader::GetWindowOut()                 { return CorrectEndianness(fWindowOut, 4); }
UInt_t GETBasicFrameHeader::GetLastCell(Int_t asadID) {
  uint8_t *tempLastCell = fLastCell[0] + 2*asadID;

  return CorrectEndianness(tempLastCell, 2);
}

ULong64_t GETBasicFrameHeader::GetFrameSkip()              { return GetFrameSize() - GetHeaderSize(); }
   UInt_t GETBasicFrameHeader::GetHeaderSkip()             { return GetHeaderSize() - GETBASICFRAMEHEADERSIZE; }

void GETBasicFrameHeader::Clear(Option_t *) {
  GETHeaderBase::Clear();

  memset( fHeaderSize, 0, sizeof(uint8_t)*  2);
  memset( fItemSize,   0, sizeof(uint8_t)*  2);
  memset( fNItems,     0, sizeof(uint8_t)*  4);
  memset( fEventTime,  0, sizeof(uint8_t)*  6);
  memset( fEventID,    0, sizeof(uint8_t)*  4);
  memset(&fCoboID,     0, sizeof(uint8_t)*  1);
  memset(&fAsadID,     0, sizeof(uint8_t)*  1);
  memset( fReadOffset, 0, sizeof(uint8_t)*  2);
  memset(&fStatus,     0, sizeof(uint8_t)*  1);
  memset( fHitPat,     0, sizeof(uint8_t)*4*9);
  memset( fMultip,     0, sizeof(uint8_t)*4*2);
  memset( fWindowOut,  0, sizeof(uint8_t)*  4);
  memset( fLastCell,   0, sizeof(uint8_t)*4*2);
}

void GETBasicFrameHeader::Read(ifstream &stream) {
  Clear();

  GETHeaderBase::Read(stream);

  stream.read((Char_t *)   fHeaderSize,   2);
  stream.read((Char_t *)     fItemSize,   2);
  stream.read((Char_t *)       fNItems,   4);
  stream.read((Char_t *)    fEventTime,   6);
  stream.read((Char_t *)      fEventID,   4);
  stream.read((Char_t *) &     fCoboID,   1);
  stream.read((Char_t *) &     fAsadID,   1);
  stream.read((Char_t *)   fReadOffset,   2);
  stream.read((Char_t *) &     fStatus,   1);
  stream.read((Char_t *)       fHitPat, 4*9);
  stream.read((Char_t *)       fMultip, 4*2);
  stream.read((Char_t *)    fWindowOut,   4);
  stream.read((Char_t *)     fLastCell, 4*2);

  stream.ignore(GetHeaderSkip());
}

void GETBasicFrameHeader::Print() {
  cout << showbase << hex;
  cout << " == GETBasicFrameHeader ========================================================================" << endl;
  cout << "    metaType: " << GetMetaType() << endl;
  cout << "              - Endianness: " << (IsLittleEndian() ? "Little" : "Big") << endl;
  cout << "              -   Blobness: " << (IsBlob() ? "YES" : "NO") << endl;
  cout << "              -  UnitBlock: " << dec << GetUnitBlock() << " Bytes" << hex << endl;
  cout << "   frameSize: " << GetFrameSize(kFALSE) << " (" << dec << GetFrameSize(kFALSE) << " Blocks = " << GetFrameSize() << hex << " Bytes)" << endl;
  cout << "  dataSource: " << GetDataSource() << endl;
  cout << "   frameType: " << GetFrameType() << endl;
  cout << "    revision: " << GetRevision() << endl;
  cout << "  headerSize: " << GetHeaderSize(kFALSE) << " (" << dec << GetHeaderSize(kFALSE) << " Blocks = " << GetHeaderSize() << hex << " Bytes)" << endl;
  cout << "    itemSize: " << GetItemSize() << endl;
  cout << "      nItems: " << GetNItems() << dec << " (" << GetNItems() << ")" << hex << endl;
  cout << "   eventTime: " << GetEventTime() << dec << " (" << GetEventTime() << ")" << endl;
  cout << "     eventID: " << GetEventID() << endl;
  cout << "      coboID: " << GetCoboID() << endl;
  cout << "      asadID: " << GetAsadID() << endl;
  cout << "  readOffset: " << GetReadOffset() << endl;
  cout << "      status: " << GetStatus() << endl;
  for (Int_t iAsad = 0; iAsad < 4; iAsad++)
    cout << "    hitPat_" << iAsad << ": " << GetHitPat(iAsad) << endl;
  for (Int_t iAsad = 0; iAsad < 4; iAsad++)
    cout << "    multip_" << iAsad << ": " << GetMultip(iAsad) << endl;
  cout << "   windowOut: " << GetWindowOut() << endl;
  for (Int_t iAsad = 0; iAsad < 4; iAsad++)
    cout << "  lastCell_" << iAsad << ": " << GetLastCell(iAsad) << endl;
  cout << " ===============================================================================================" << endl;
}
