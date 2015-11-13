#include "GETBasicFrame.hh"

GETBasicFrame::GETBasicFrame() {
  Clear();
}

Int_t *GETBasicFrame::GetSample(Int_t agetIdx, Int_t chIdx) { return fSample + GetIndex(agetIdx, chIdx, 0); }

Int_t GETBasicFrame::GetFrameSkip() { return GetFrameSize() - GETBASICFRAMEHEADERSIZE - GetHeaderSkip() - GetItemSize()*GetNItems(); }

void GETBasicFrame::Clear(Option_t *) {
  GETBasicFrameHeader::Clear();

  memset(fSample, 0, sizeof(Int_t)*4*68*512);
}

void GETBasicFrame::Read(ifstream &stream) {
  Clear();

  GETBasicFrameHeader::Read(stream);

  if (GetFrameType() == GETFRAMEBASICTYPE1) {
    uint8_t data[4];
    for (Int_t iItem = 0; iItem < GetNItems(); iItem++) {
      stream.read((Char_t *) data, GetItemSize());

      UInt_t item = CorrectEndianness(data, 4);

      UShort_t agetIdx = ((item & 0xc0000000) >> 30);
      UShort_t   chIdx = ((item & 0x3f800000) >> 23);
      UShort_t   tbIdx = ((item & 0x007fc000) >> 14);
      UShort_t  sample =  (item & 0x00000fff);         

      fSample[GetIndex(agetIdx, chIdx, tbIdx)] = sample; 
    }
  } else if (GetFrameType() == GETFRAMEBASICTYPE2) {
    uint8_t data[2];
    for (Int_t iItem = 0; iItem < GetNItems(); iItem++) {
      stream.read((Char_t *) data, GetItemSize());

      UShort_t item = CorrectEndianness(data, 2);

      UShort_t agetIdx = ((item & 0xc000) >> 14);
      UShort_t   chIdx = ((iItem/8)*2 + iItem%2)%68;
      UShort_t   tbIdx = iItem/(68*4);
      UShort_t  sample = item & 0x0fff;

      fSample[GetIndex(agetIdx, chIdx, tbIdx)] = sample; 
    }
  }

  stream.ignore(GetFrameSkip());
}

UInt_t GETBasicFrame::GetIndex(Int_t agetIdx, Int_t chIdx, Int_t tbIdx) { return agetIdx*68*512 + chIdx*512 + tbIdx; }
