#ifndef STPULSE
#define STPULSE

#include "TF1.h"
#include "Rtypes.h"

class STPulse
{
  public:
    STPulse();
    ~STPulse() {}

    Double_t PulseF1(Double_t *x, Double_t *par);
    Double_t Pulse(Double_t x, Double_t amp, Double_t tb);

    TF1* GetPulseFunction(TString name);

  private:
    Double_t *fPulseData;

  ClassDef(STPulse, 1)
};

#endif
