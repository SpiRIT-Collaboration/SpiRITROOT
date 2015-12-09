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
    void FindHits(STPad *pad, TClonesArray *hitArray, Int_t &hitNum);

  private:
    void FitPulse(Double_t *buffer, Double_t tbStart, Int_t ndf, Double_t &chi2, Double_t &amplitude);

  private:
    TClonesArray **fThreadHitArray; /// TClonesArray object

    Bool_t fPadReady;
    Bool_t fPadTaken;
    Bool_t fEnd;

    std::mutex fMutex;
    std::condition_variable fCondVariable;

    STPad *fPad;

    Int_t fNDFTbs;
    Int_t fIterMax;
    Int_t fNumTbsCorrection;

  ClassDef(STPSAFastFit, 1)
};

#endif
