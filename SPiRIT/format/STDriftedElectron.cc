//---------------------------------------------------------------------
// Description:
//      Drifted Electron data class source
//
//      Data class for drifted elctron STDriftTask.
//      Drifted electron is electrons drifted through gas
//      with diffusions
//
//      Created by : STDriftTask
//      Used    by : STAvalacheTask
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
  fTime(0)
{
}

STDriftedElectron::STDriftedElectron(Double_t x,
                                     Double_t z,
                                     Double_t time)
: fX(x),
  fZ(z),
  fTime(time)
{
}

STDriftedElectron::~STDriftedElectron()
{
}

// Getters
Double_t STDriftedElectron::GetX()    { return fX; }
Double_t STDriftedElectron::GetZ()    { return fZ; }
Double_t STDriftedElectron::GetTime() { return fTime; }

// Setters
void     STDriftedElectron::SetIndex(Int_t index) { fIndex = index; }
