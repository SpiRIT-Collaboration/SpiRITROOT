//-----------------------------------------------------------
// Description:
//   Just for providing operator....
//   Wow header is longer than its contents!
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STHitSortY.hh"

// ROOT classes
#include "TVector3.h"

STHitSortY::STHitSortY()  {}
STHitSortY::~STHitSortY() {}

Bool_t
STHitSortY::operator()(STHit hitA, STHit hitB)
{
  return (hitA.GetPosition().Y() < hitB.GetPosition().Y());
}
