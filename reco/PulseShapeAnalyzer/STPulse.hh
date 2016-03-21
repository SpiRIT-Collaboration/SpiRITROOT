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
    STPulse(TString fileName);

    ~STPulse() {}

    /** Get Pulse value in x position with parameter (amp, tb0) */
    Double_t Pulse(Double_t x, Double_t amp, Double_t tb0);

    /** Get Pulse with inialtial parameter (0,0) */
    TF1* GetPulseFunction(TString name = "");

    /** Get Pulse with STHit information */
    TF1* GetPulseFunction(STHit* hit, TString name = "");

    Double_t  GetTbAtThreshold();
    Double_t  GetTbAtMax();
       Int_t  GetNumAscending();
    Double_t  GetThresholdTbStep();
       Int_t  GetNumDataPoints();
    Double_t *GetPulseData();

    void Print();

  private:
    /** Initialize data and parameters. */
    void Initialize(TString fileName);

    /** A general C++ function object (functor) with parameters */
    Double_t PulseF1(Double_t *x, Double_t *par);

    /** The Pulse data points Will be updated as STPulse is initialized. */
    Double_t *fPulseData;

    /** Number of data points. Will be updated as STPulse is initialized. */
    Int_t fNumDataPoints;

    /** 
     * Step of data points in 1 time-bucket unit for current data file.
     * Should be smaller than 1.
     * The data points are parted by (fStep * [time-bucket]) from each other.
     */
    Double_t fStep;

    /** Ratio height compare to peak height where pulse starts to rise **/
    Double_t fThresholdRatio = 0.05;

    /** Number of the pulse function(TF1*) created by this class */
    Int_t fNumF1;

  protected:
    /** 
     * Time-bucket at threshold-ratio of peak from start of the pulse.
     * Will be updated as STPulse is initialized.
     */
    Double_t fTbAtThreshold;

    /** 
     * Time-bucket at peak from start of the pulse.
     * Will be updated as STPulse is initialized.
     */
    Double_t fTbAtMax;

    /** Number of timebucket while rising **/
    Int_t fNumAscending;

    /** Threshold of one timebucket step while risiing **/
    Double_t fThresholdTbStep;

  ClassDef(STPulse, 3)
};

#endif
