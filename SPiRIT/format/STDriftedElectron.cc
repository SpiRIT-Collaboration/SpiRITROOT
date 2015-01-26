/**
 * @brief Information of electrons produced in STDriftTask.
 *
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STDriftedElectron.hh"

ClassImp(STDriftedElectron);

STDriftedElectron::STDriftedElectron()
: fX(0),
  fDiffusedX(0),
  fZ(0),
  fDiffusedZ(0),
  fHitY(0),
  fHitTime(0),
  fDriftTime(0),
  fDiffusedTime(0),
  fIWire(0),
  fZWire(0),
  fGain(0)
{
}

STDriftedElectron::STDriftedElectron(Double_t x,
                                     Double_t diffusedX,
                                     Double_t z,
                                     Double_t diffusedZ,
                                     Double_t hitY,
                                     Double_t hitTime,
                                     Double_t driftTime,
                                     Double_t diffusedTime,
                                     Int_t    iWire,
                                     Int_t    zWire,
                                     Int_t    gain)
: fX(x),
  fDiffusedX(diffusedX),
  fZ(z),
  fDiffusedZ(diffusedZ),
  fHitY(hitY),
  fHitTime(hitTime),
  fDriftTime(driftTime),
  fDiffusedTime(diffusedTime),
  fIWire(iWire),
  fZWire(zWire),
  fGain(gain)
{
}

STDriftedElectron::~STDriftedElectron()
{
}

// Getters
Double_t STDriftedElectron::GetX()            { return fX; }
Double_t STDriftedElectron::GetDiffusedX()    { return fDiffusedX; }
Double_t STDriftedElectron::GetZ()            { return fZ; }
Double_t STDriftedElectron::GetDiffusedZ()    { return fDiffusedZ; }
Double_t STDriftedElectron::GetHitY()         { return fHitY; }
Double_t STDriftedElectron::GetHitTime()      { return fHitTime; }
Double_t STDriftedElectron::GetDriftTime()    { return fDriftTime; }
Double_t STDriftedElectron::GetDiffusedTime() { return fDiffusedTime; }
   Int_t STDriftedElectron::GetIWire()        { return fIWire; }
   Int_t STDriftedElectron::GetZWire()        { return fZWire; }
   Int_t STDriftedElectron::GetGain()         { return fGain; }

// Setters
void STDriftedElectron::SetIndex(Int_t index) { fIndex = index; }
