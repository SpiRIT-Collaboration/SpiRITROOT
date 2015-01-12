//---------------------------------------------------------------------
// Description:
//      Digitized Electron data class source
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STDigitizedElectron.hh"

ClassImp(STDigitizedElectron);

STDigitizedElectron::STDigitizedElectron()
: fX(0),
  fZ(0),
  fZWire(0),
  fTime(0),
  fGain(0)
{
}

STDigitizedElectron::STDigitizedElectron(Double_t x,
                                         Double_t z,
                                         Double_t zWire,
                                         Double_t time,
                                         Int_t    gain)
: fX(x),
  fZ(z),
  fZWire(zWire),
  fTime(time),
  fGain(gain)
{
}

STDigitizedElectron::~STDigitizedElectron()
{
}

// Getters
Double_t STDigitizedElectron::GetX()     { return fX; }
Double_t STDigitizedElectron::GetZ()     { return fZ; }
Int_t    STDigitizedElectron::GetZWire() { return fZWire; }
Double_t STDigitizedElectron::GetTime()  { return fTime; }
Int_t    STDigitizedElectron::GetGain()  { return fGain; }

// Setters
void STDigitizedElectron::SetIndex(Int_t index) { fIndex = index; }
