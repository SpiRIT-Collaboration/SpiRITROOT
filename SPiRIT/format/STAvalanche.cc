//---------------------------------------------------------------------
// Description:
//      Avalanche data class source
//
//      Data class for drifted elctron STAvalacheTask.
//
//      Created by : STAvalancheTask
//      Used    by : STPadResponseTask
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STAvalanche.hh"

ClassImp(STAvalanche);

STAvalanche::STAvalanche()
: fX(0),
  fZ(0),
  fTime(0),
  fGain(0)
{
}

STAvalanche::STAvalanche(Double_t x,
                         Double_t z,
                         Double_t time,
                         Double_t gain)
: fX(x),
  fZ(z),
  fTime(time),
  fGain(gain)
{
}

STAvalanche::~STAvalanche()
{
}

// Getters
Double_t STAvalanche::GetX()    { return fX; }
Double_t STAvalanche::GetZ()    { return fZ; }
Double_t STAvalanche::GetTime() { return fTime; }
Double_t STAvalanche::GetGain() { return fGain; }

// Setters
void     STAvalanche::SetIndex(Int_t index) { fIndex = index; }
