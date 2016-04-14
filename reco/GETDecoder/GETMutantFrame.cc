#include "GETMutantFrame.hh"

ClassImp(GETMutantFrame)

GETMutantFrame::GETMutantFrame() {
  Clear();
}

ULong64_t GETMutantFrame::GetTimestamp()             { return CorrectEndianness(fTimestamp, 6); }
   UInt_t GETMutantFrame::GetEventNumber()           { return CorrectEndianness(fEventNumber, 4); }
   UInt_t GETMutantFrame::GetTriggerInfo()           { return CorrectEndianness(fTriggerInfo, 2); }
   UInt_t GETMutantFrame::GetMultiplicity(Int_t id)  { return CorrectEndianness(fMultiplicity + id*2, 2); }
   UInt_t GETMutantFrame::GetEventCounter(Int_t id)  { return CorrectEndianness(fEventCounter + id*4, 4); }
   UInt_t GETMutantFrame::GetScaler(Int_t id)        { return CorrectEndianness(fScaler + id*4, 4); }
   UInt_t GETMutantFrame::GetD2PTime()               { return CorrectEndianness(fD2PTime, 4); }

ULong64_t GETMutantFrame::GetFrameSkip()  { return 0; }
ULong64_t GETMutantFrame::GetHeaderSkip() { return 0; }

void GETMutantFrame::Clear(Option_t *) {
  GETHeaderBase::Clear();

  memset(fTimestamp,    0, sizeof(uint8_t)* 6);
  memset(fEventNumber,  0, sizeof(uint8_t)* 4);
  memset(fTriggerInfo,  0, sizeof(uint8_t)* 2);
  memset(fMultiplicity, 0, sizeof(uint8_t)* 4);
  memset(fEventCounter, 0, sizeof(uint8_t)*16);
  memset(fScaler,       0, sizeof(uint8_t)*20);
  /*
  memset(fMultiplicityA,   0, sizeof(uint8_t)*2);
  memset(fMultiplicityB,   0, sizeof(uint8_t)*2);
  memset(fL0EventCounter,  0, sizeof(uint8_t)*2);
  memset(fL1AEventCounter, 0, sizeof(uint8_t)*2);
  memset(fL1BEventCounter, 0, sizeof(uint8_t)*2);
  memset(fL2EventCounter,  0, sizeof(uint8_t)*2);
  memset(fScaler1,         0, sizeof(uint8_t)*2);
  memset(fScaler2,         0, sizeof(uint8_t)*2);
  memset(fScaler3,         0, sizeof(uint8_t)*2);
  memset(fScaler4,         0, sizeof(uint8_t)*2);
  memset(fScaler5,         0, sizeof(uint8_t)*2);
  */
  memset(fD2PTime,      0, sizeof(uint8_t)* 4);
}

void GETMutantFrame::Read(ifstream &stream) {
  Clear();

  GETHeaderBase::Read(stream);

  stream.read((Char_t *) fTimestamp,         6);
  stream.read((Char_t *) fEventNumber,       4);
  stream.read((Char_t *) fTriggerInfo,       2);
  stream.read((Char_t *) fMultiplicity,      2);
  stream.read((Char_t *) fMultiplicity +  2, 2);
  stream.read((Char_t *) fEventCounter,      4);
  stream.read((Char_t *) fEventCounter +  4, 4);
  stream.read((Char_t *) fEventCounter +  8, 4);
  stream.read((Char_t *) fEventCounter + 12, 4);
  stream.read((Char_t *) fScaler,            4);
  stream.read((Char_t *) fScaler + 4,        4);
  stream.read((Char_t *) fScaler + 8,        4);
  stream.read((Char_t *) fScaler + 12,       4);
  stream.read((Char_t *) fScaler + 16,       4);
  /*
  stream.read((Char_t *) fMultiplicityA,   2);
  stream.read((Char_t *) fMultiplicityB,   2);
  stream.read((Char_t *) fL0EventCounter,  2);
  stream.read((Char_t *) fL1AEventCounter, 2);
  stream.read((Char_t *) fL1BEventCounter, 2);
  stream.read((Char_t *) fL2EventCounter,  2);
  stream.read((Char_t *) fScaler1,         2);
  stream.read((Char_t *) fScaler2,         2);
  stream.read((Char_t *) fScaler3,         2);
  stream.read((Char_t *) fScaler4,         2);
  stream.read((Char_t *) fScaler5,         2);
  */
  stream.read((Char_t *) fD2PTime,         4);
}

void GETMutantFrame::Print() {
  cout << showbase << hex;
  cout << " == GETMutantFrame =============================" << endl;
  cout << "          metaType: " << GetMetaType() << endl;
  cout << "                    - Endianness: " << (IsLittleEndian() ? "Little" : "Big") << endl;
  cout << "                    -   Blobness: " << (IsBlob() ? "YES" : "NO") << endl;
  cout << "                    -  UnitBlock: " << dec << GetUnitBlock() << " Bytes" << hex << endl;
  cout << "         frameSize: " << GetFrameSize(false) << " (" << dec << GetFrameSize(false) << " Blocks = " << GetFrameSize() << hex << " Bytes)" << endl;
  cout << "        dataSource: " << GetDataSource() << endl;
  cout << "         frameType: " << GetFrameType() << endl;
  cout << "          revision: " << GetRevision() << endl;
  cout << "         timestamp: " << GetTimestamp() << dec << " (" << GetTimestamp() << ")" << endl;
  cout << "       eventNumber: " << GetEventNumber() << endl;
  cout << "       triggerInfo: " << hex << GetTriggerInfo() << dec << " (Below are binary)" << endl;
  cout << "                    -        Master: " << ((GetTriggerInfo()&0xc000 >> 15) == 0) << endl;
  cout << "                    -       Slave 1: " << ((GetTriggerInfo()&0xc000 >> 15) == 1) << endl;
  cout << "                    -       Slave 2: " << ((GetTriggerInfo()&0xc000 >> 15) == 2) << endl;
  cout << "                    -  L1A_LOW_TRIG: " << (GetTriggerInfo()&0x1) << endl;
  cout << "                    - L1A_HIGH_TRIG: " << (GetTriggerInfo()&0x2 >> 1) << endl;
  cout << "                    -  L1B_LOW_TRIG: " << (GetTriggerInfo()&0x4 >> 2) << endl;
  cout << "                    - L1A_HIGH_TRIG: " << (GetTriggerInfo()&0x8 >> 3) << endl;
  cout << "                    -          L0EN: " << (GetTriggerInfo()&0x10 >> 4) << endl;
  cout << "                    -          L1EN: " << (GetTriggerInfo()&0x20 >> 5) << endl;
  cout << "                    -          L2EN: " << (GetTriggerInfo()&0x40 >> 6) << endl;
  cout << "                    -           L2W: " << (GetTriggerInfo()&0x80 >> 7) << endl;
  cout << "                    -         L0/L1: " << (GetTriggerInfo()&0x100 >> 8) << endl;
  cout << "                    -         L1SEL: " << (GetTriggerInfo()&0x400 >> 10) << (GetTriggerInfo()&0x200 >> 9) << " (binary)" << endl;
  cout << "                    -           TPD: " << (GetTriggerInfo()&0x800 >> 11) << endl;
  cout << "                    -           HER: " << (GetTriggerInfo()&0x1000 >> 12) << endl;
  cout << "    multiplicity A: " << GetMultiplicity(0) << endl;
  cout << "    multiplicity B: " << GetMultiplicity(1) << endl;
  cout << " L0  Event Counter: " << GetEventCounter(0) << endl;
  cout << " L1A Event Counter: " << GetEventCounter(1) << endl;
  cout << " L1B Event Counter: " << GetEventCounter(2) << endl;
  cout << " L2  Event Counter: " << GetEventCounter(3) << endl;
  cout << "          scaler 1: " << GetScaler(0) << endl;
  cout << "          scaler 2: " << GetScaler(1) << endl;
  cout << "          scaler 3: " << GetScaler(2) << endl;
  cout << "          scaler 4: " << GetScaler(3) << endl;
  cout << "          scaler 5: " << GetScaler(4) << endl;
  cout << "          D2P Time: " << hex << GetD2PTime() << dec << " (" << GetD2PTime() << ") " << endl;
  cout << " ===============================================" << endl;
}
