/**
 * @brief Find all peaks and make each of them into hits.
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STPSAFASTFIT
#define STPSAFASTFIT

// SpiRITROOT classes
#include "STPSA.hh"

// ROOT classes
#include "TSpectrum.h"
#include "TClonesArray.h"
#include "TF1.h"

// STL
#include <mutex>
#include <condition_variable>

class STPSAFastFit : public STPSA
{
  public:
    STPSAFastFit();

    void Analyze(STRawEvent *rawEvent, STEvent *event);
    void PadAnalyzer(TClonesArray *hitArray);

  private:
    Double_t Pulse(Double_t *x, Double_t *par);
    void FitPulse(Double_t *buffer, Double_t tbStart, Double_t &chi2, Double_t &amp);

  private:
    TClonesArray **fThreadHitArray; /// TClonesArray object

    Bool_t fPadReady;
    Bool_t fPadTaken;
    Bool_t fEnd;

    std::mutex fMutex;
    std::condition_variable fCondVariable;

    STPad *fPad;

    Double_t *fPulseData;
    TF1 *fPulse;

    Int_t fNumTbsCompare;

  ClassDef(STPSAFastFit, 1)
};

#endif
