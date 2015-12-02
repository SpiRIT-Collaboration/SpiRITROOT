#ifndef STPULSE
#define STPULSE

#include "Rtypes.h"

class STPulse
{
  public:
    STPulse();
    ~STPulse() {}

    Double_t Pulse(Double_t x, Double_t amp, Double_t tb);
    Double_t Pulse(Double_t *x, Double_t *par);

  private:
    Double_t *fPulseData;


  ClassDef(STPulse, 1)
};

#endif
