#ifndef STPULSE
#define STPULSE

#include "TF1.h"
#include "Rtypes.h"
#include "STHit.hh"
#include "STSamplePoint.hh"
#include "TGraph.h"

class STPulse
{
  public:
    /** Construct STPulse with default setting */
    STPulse();

    /**
     * Construct STPulse with fileName and step.
     * step is the difference of the time-bucket between data points.
     * See fStepSize.
     */
    STPulse(TString fileName);

    STPulse(Int_t shapingTime);

    ~STPulse() {}

    /** Get Pulse value in x position with parameter (amp, tb0) */
    Double_t Pulse(Double_t x, Double_t amp, Double_t tb0);

    /** Get Pulse with inialtial parameter (0,0) */
    TF1* GetPulseFunction(TString name = "");

    /** Get Pulse with STHit information */
    TF1* GetPulseFunction(STHit* hit, TString name = "");

       Int_t  GetShapingTime();
    Double_t  GetTbAtThreshold();
    Double_t  GetTbAtTail();
    Double_t  GetTbAtMax();
       Int_t  GetNumAscending();
    Double_t  GetThresholdTbStep();
       Int_t  GetNumDataPoints();
    Double_t  GetStepSize();

    STSamplePoint **GetPulseData();

    void Print();

    void SavePulseData(TString name, Bool_t smoothTail = true);

  private:
    /** Initialize data and parameters. */
    void Initialize(TString fileName);

    /** A general C++ function object (functor) with parameters */
    Double_t PulseF1(Double_t *x, Double_t *par);

    /** The Pulse data points Will be updated as STPulse is initialized. */
    STSamplePoint *fPulseData;

    /** Shaping time. */
    Int_t fShapingTime;

    /** Number of data points. Will be updated as STPulse is initialized. */
    Int_t fNumDataPoints;

    /** 
     * Step of data points in 1 time-bucket unit for current data file.
     * Should be smaller than 1.
     * The data points are parted by (fStepSize * [time-bucket]) from each other.
     */
    Double_t fStepSize;

    /** Ratio height compare to peak height where pulse starts to rise **/
    Double_t fThresholdRatio = 0.05;

    /** Number of the pulse function(TF1*) created by this class */
    Int_t fNumF1 = 0;

  protected:
    /** 
     * Time-bucket at threshold-ratio of peak from start of the pulse.
     * Will be updated as STPulse is initialized.
     */
    Double_t fTbAtThreshold;

    /** Time-bucket just before the tail starts. */
    Double_t fTbAtTail;

    /** 
     * Time-bucket at peak from start of the pulse.
     * Will be updated as STPulse is initialized.
     */
    Double_t fTbAtMax;

    /** Number of timebucket while rising **/
    Int_t fNumAscending;

    /** Threshold of one timebucket step while risiing **/
    Double_t fThresholdTbStep;

    /**
     * Number of degree of freedom on time-bucket when performing
     * least square fit of amplitude of the pulse using LSFitPulse().
     */
    Int_t fNDFTbs;

  private:
    TF1 *fTailFunction = nullptr;

    TGraph *fTailGraph = nullptr;

  ClassDef(STPulse, 4)
};

#endif
