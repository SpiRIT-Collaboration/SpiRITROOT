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

#ifndef STPSASIMPLE
#define STPSASIMPLE

#include "STPSA.hh"

class STPSASimple : public STPSA
{
  public:
    STPSASimple();
    ~STPSASimple();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  ClassDef(STPSASimple, 1)
};

#endif
