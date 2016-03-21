/**
 * @brief Find all peaks and make each of them into hits.
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STPSAFASTFIT
#define STPSAFASTFIT

// SpiRITROOT classes
#include "STPSA.hh"
#include "STPulse.hh"

// ROOT classes
#include "TSpectrum.h"
#include "TClonesArray.h"

// STL
#include <mutex>
#include <condition_variable>

class STPSAFastFit : public STPSA, public STPulse
{
  public:
    STPSAFastFit();

    void Analyze(STRawEvent *rawEvent, STEvent *event);
    void PadAnalyzer(TClonesArray *hitArray);

    /** 
     * Find hits from the pad, pass hits to hitArray
     * Process is done as below:
     *  1. FindPeak()
     *  2. FitPulse()
     *     - LSFitPulse()
     *  3. LSFitPulse()
     */
    void FindHits(STPad *pad, TClonesArray *hitArray, Int_t &hitNum);

    /**
     * Find the first peak from adc time-bucket starting from input tbCurrent
     * tbCurrent and tbStart becomes time-bucket of the peak and starting point
     */
    Bool_t FindPeak(Double_t *adc, Int_t &tbCurrent, Int_t &tbStart);

    /**
     * Perform least square fitting with the the pulse around tbStart ~ tbPeak.
     * This process is Iteration based process using method LSFitPuse();
     */
    Bool_t FitPulse(Double_t *adc, Int_t tbStart, Int_t tbPeak,
                    Double_t &tbHit, Double_t &amplitude, 
                    Double_t &squareSum, Int_t &ndf);

    /**
     * Perform least square fitting with the fixed parameter tbStart and ndf.
     * This process is analytic. The amplitude is choosen imidiatly.
     */
    void LSFitPulse(Double_t *buffer, Double_t tbStart, 
                    Int_t ndf, Double_t &chi2, Double_t &amplitude);

    /**
     * Test pulse with previous pulse and currently found pulse.
     * Returns true is current pulse is distinguished to be real pulse
     */
    Bool_t TestPulse(Double_t *adc, Double_t tbHitPre, Double_t amplitudePre, 
                     Double_t tbHit, Double_t amplitude);

  private:
    TClonesArray **fThreadHitArray; ///< TClonesArray object for thread

    Bool_t fPadReady;
    Bool_t fPadTaken;
    Bool_t fEnd;

    std::mutex fMutex;
    std::condition_variable fCondVariable;

    STPad *fPad;

    /**
     * Number of degree of freedom on time-bucket when performing
     * least square fit of amplitude of the pulse using LSFitPulse().
     */
    Int_t fNDFTbs = 12;
    Int_t fTbStartCut;

    /**
     * Maximum number of iteration alowed for fitting parameter tbStart.
     * Where, tbStart is "staring time-bucket of the pulse"
     */
    Int_t fIterMax = 40;

    /** 
     * Number of time-bucket to subtract fitted pulse from the adc distribution.
     *
     * This means:
     * for (tb = tbStart; tb < tbStart + fNumTbsCorrection; tb ++)
     *   [adc distribution]_tb -= [fitted pulse]_tb
     *
     * Note that adc distribution will be used agian to find the next peak.
     */
    Int_t fNumTbsCorrection = 50;

    /**
     * Proportional parameter for the time-bucket step when fitting tbStart.
     *
     * Each time the least square fit is performed for amplitude of the pulse
     * with fixed tbStart, we have to select next candidate of the tbStart. 
     * The step of time-bucket from current tbStart is choosen as
     * step = fAlpha / peak^2 * beta.
     *
     * for beta, see fBetaCut.
     */
    Double_t fAlpha = 20.;

    /**
     * The default cut for beta. If beta becomes lower than fBetaCut
     * for twice in a row, the fit is satisfied.
     *
     * beta is defined by
     *   minus of [difference between least-squares of previous fit and current fit] 
     *   divided by [difference between time-bucket of previous fit and current fit] 
     *   divided by NDF: 
     *   beta = -(lsCur-lsPre)/(tbCur-tbPre)/ndf.
     *
     * This cut is redefined as effective threhold for each pulses 
     * betaCut = fBetaCut * peak^2
     */
    Double_t fBetaCut = 1.e-3; 

  ClassDef(STPSAFastFit, 2)
};

#endif
