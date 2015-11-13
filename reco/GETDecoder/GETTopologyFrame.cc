#include "GETTopologyFrame.hh"

ClassImp(GETTopologyFrame)

GETTopologyFrame::GETTopologyFrame() {
  Clear();
}

   UInt_t GETTopologyFrame::GetCoboIdx()    { return (UInt_t) fCoboIdx; }
bitset<4> GETTopologyFrame::GetAsadMask()   { return bitset<4>((UInt_t) fAsadMask); }
   UInt_t GETTopologyFrame::Get2pMode()     { return (UInt_t) f2pMode; }
   UInt_t GETTopologyFrame::GetUNUSED()     { return (UInt_t) fUNUSED; }
ULong64_t GETTopologyFrame::GetFrameSkip()  { return 0; }
ULong64_t GETTopologyFrame::GetHeaderSkip() { return 0; }

void GETTopologyFrame::Clear(Option_t *) {
  GETHeaderBase::Clear();

  memset(&fCoboIdx,  0, sizeof(uint8_t));
  memset(&fAsadMask, 0, sizeof(uint8_t));
  memset(&f2pMode,   0, sizeof(uint8_t));
  memset(&fUNUSED,   0, sizeof(uint8_t));
}

void GETTopologyFrame::Read(ifstream &stream) {
  Clear();

  GETHeaderBase::Read(stream);

  stream.read((Char_t *) &  fCoboIdx, 1);
  stream.read((Char_t *) & fAsadMask, 1);
  stream.read((Char_t *) &   f2pMode, 1);
  stream.read((Char_t *) &   fUNUSED, 1);
}

void GETTopologyFrame::Print() {
  cout << showbase << hex;
  cout << " == GETTopologyFrame =======================" << endl;
  cout << "    metaType: " << GetMetaType() << endl;
  cout << "              - Endianness: " << (IsLittleEndian() ? "Little" : "Big") << endl;
  cout << "              -   Blobness: " << (IsBlob() ? "YES" : "NO") << endl;
  cout << "              -  UnitBlock: " << dec << GetUnitBlock() << " Bytes" << hex << endl;
  cout << "   frameSize: " << GetFrameSize(false) << " (" << dec << GetFrameSize(false) << " Blocks = " << GetFrameSize() << hex << " Bytes)" << endl;
  cout << "  dataSource: " << GetDataSource() << endl;
  cout << "   frameType: " << GetFrameType() << endl;
  cout << "    revision: " << GetRevision() << endl;
  cout << "     coboIdx: " << GetCoboIdx() << endl;
  cout << "    asadMask: " << GetAsadMask() << " (" << bitset<4>(GetAsadMask()) << ")" << endl;
  cout << "      2pMode: " << Get2pMode() << endl;
  cout << "      UNUSED: " << GetUNUSED() << endl;
  cout << " ===========================================" << endl;
}
