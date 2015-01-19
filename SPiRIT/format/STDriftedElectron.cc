//---------------------------------------------------------------------
// Description:
//      Digitized Electron data class source
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STDriftedElectron.hh"

ClassImp(STDriftedElectron);

STDriftedElectron::STDriftedElectron()
: fX(0),
  fZ(0),
  fZWire(0),
  fTime(0),
  fGain(0)
{
}

STDriftedElectron::STDriftedElectron(Double_t x,
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

STDriftedElectron::~STDriftedElectron()
{
}

// Getters
Double_t STDriftedElectron::GetX()     { return fX; }
Double_t STDriftedElectron::GetZ()     { return fZ; }
   Int_t STDriftedElectron::GetZWire() { return fZWire; }
Double_t STDriftedElectron::GetTime()  { return fTime; }
   Int_t STDriftedElectron::GetGain()  { return fGain; }

// Setters
void STDriftedElectron::SetIndex(Int_t index) { fIndex = index; }
