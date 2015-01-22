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
  fDX(0),
  fZ(0),
  fDZ(0),
  fT(0),
  fDT(0),
  fIWire(0),
  fZWire(0),
  fGain(0)
{
}

STDriftedElectron::STDriftedElectron(Double_t x,
                                     Double_t dx,
                                     Double_t z,  
                                     Double_t dz,
                                     Double_t t,
                                     Double_t dt,
                                     Int_t    iWire,
                                     Int_t    zWire,
                                     Int_t    gain)
: fX(x),
  fDX(dx),
  fZ(z),
  fDZ(dz),
  fT(t),
  fDT(dt),
  fIWire(iWire),
  fZWire(zWire),
  fGain(gain)
{
}

STDriftedElectron::~STDriftedElectron()
{
}

// Getters
Double_t STDriftedElectron::GetX()     { return fX; }
Double_t STDriftedElectron::GetDX()    { return fDX; }
Double_t STDriftedElectron::GetZ()     { return fZ; }
Double_t STDriftedElectron::GetDZ()    { return fDZ; }
Double_t STDriftedElectron::GetT()  { return fT; }
Double_t STDriftedElectron::GetDT() { return fDT; }
   Int_t STDriftedElectron::GetIWire() { return fIWire; }
   Int_t STDriftedElectron::GetZWire() { return fZWire; }
   Int_t STDriftedElectron::GetGain()  { return fGain; }

// Setters
void STDriftedElectron::SetIndex(Int_t index) { fIndex = index; }
