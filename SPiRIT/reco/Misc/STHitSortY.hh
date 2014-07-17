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

#ifndef STHITSORTY_H
#define STHITSORTY_H

// SpiRITROOT classes
#include "STHit.hh"

// ROOT classes
#include "Rtypes.h"

class STHitSortY
{
  public:
    STHitSortY();
    ~STHitSortY();

    Bool_t operator()(STHit hitA, STHit hitB);
};

#endif
