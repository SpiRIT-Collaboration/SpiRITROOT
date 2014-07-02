//-----------------------------------------------------------
// Description:
//   Simple version of analyzing pulse shape of raw signal.
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef STPSASIMPLE_H
#define STPSASIMPLE_H

// SpiRITROOT classes
#include "STRawEvent.hh"
#include "STPad.hh"

// FairRoot classes
#include "FairRootManager.h"

// STL
#include <vector>

// ROOT classes
#include "TClonesArray.h"

class STPSASimple
{
  public:
    STPSASimple();
    ~STPSASimple();

  private:

  ClassDef(STPSASimple, 1)
};

#endif
