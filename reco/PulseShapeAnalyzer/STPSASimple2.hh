//-----------------------------------------------------------
// Description:
//   Simple version 2 of analyzing pulse shape of raw signal.
//   This version uses TSpectrum class in ROOT to find
//   peaks in pads.
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#pragma once

// SpiRITROOT classes
#include "STPSA.hh"

// ROOT classes
#include "TSpectrum.h"

class STPSASimple2 : public STPSA
{
  public:
    STPSASimple2();
    ~STPSASimple2();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  private:
    TSpectrum *fPeakFinder;  /// TSpectrum object

  ClassDef(STPSASimple2, 1)
};
