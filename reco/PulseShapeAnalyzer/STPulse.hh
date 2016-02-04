#ifndef STPULSE
#define STPULSE

#include "TF1.h"
#include "Rtypes.h"
#include "STHit.hh"

class STPulse
{
  public:
    /** Construct STPulse with default setting */
    STPulse();

    /**
     * Construct STPulse with fileName and step.
     * step is the difference of the time-bucket between data points.
     * See fStep.
     */
    STPulse(TString fileName, Double_t step = 0.1);

    ~STPulse() {}

    /** Get value of starting point of the pulse where the peak value is 1 */
    Double_t GetStartValue() { return fPulseData[0]; }

    /** Get time-bucket difference from start of the pulse to the peak */
    Double_t GetTbAtMax() { return fTbAtMax; }

    /** Get Pulse value in x position with parameter (amp, tb0) */
    Double_t Pulse(Double_t x, Double_t amp, Double_t tb0);

    /** Get Pulse with inialtial parameter (0,0) */
    TF1* GetPulseFunction(TString name = "");

    /** Get Pulse with STHit information */
    TF1* GetPulseFunction(STHit* hit, TString name = "");

  private:
    /** Initialize data and parameters. */
    void Initialize(TString fileName);

    /** A general C++ function object (functor) with parameters */
    Double_t PulseF1(Double_t *x, Double_t *par);

    /** Number of data points. Will be updated as STPulse is initialized. */
    Int_t fNumDataPoint = 0;

    /** The Pulse data points Will be updated as STPulse is initialized. */
    Double_t *fPulseData;

    /** 
     * Time-bucket difference from start of the pulse to the peak.
     * Will be updated as STPulse is initialized.
     */
    Double_t fTbAtMax = 0; 

    /** 
     * Step of data points in 1 time-bucket unit for current data file.
     * Should be smaller than 1.
     * The data points are parted by (fStep * [time-bucket]) from each other.
     */
    Double_t fStep = 0.1;

    /** Number of the pulse function(TF1*) created by this class */
    Int_t fNumF1 = 0;

  ClassDef(STPulse, 2)
};

#endif
