#ifndef STPSAFAST_HH
#define STPSAFAST_HH

#include "STPSA.hh"

class STPSAFast : public STPSA
{
  public:
    STPSAFast();
    ~STPSAFast();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  ClassDef(STPSAFast, 0)
};

#endif
