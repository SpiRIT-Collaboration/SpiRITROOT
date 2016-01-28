#ifndef STPULSE
#define STPULSE

#include "TF1.h"
#include "Rtypes.h"
#include "STHit.hh"

class STPulse
{
  public:
    STPulse();
    ~STPulse() {}

    Double_t PulseF1(Double_t *x, Double_t *par);
    Double_t Pulse(Double_t x, Double_t amp, Double_t tb);

    TF1* GetPulseFunction(TString name);
    TF1* GetPulseFunction(STHit* hit, TString name = "");

  private:
    Double_t *fPulseData;
    Int_t fIndex = 0;

  ClassDef(STPulse, 1)
};

#endif
