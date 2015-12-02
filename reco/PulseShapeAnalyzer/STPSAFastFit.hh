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

    void SetNumTbsCompare

  private:
    void FitPulse(Double_t *buffer, Double_t tbStart, Double_t &chi2, Double_t &amp);

  private:
    TClonesArray **fThreadHitArray; /// TClonesArray object

    Bool_t fPadReady;
    Bool_t fPadTaken;
    Bool_t fEnd;

    std::mutex fMutex;
    std::condition_variable fCondVariable;

    STPad *fPad;

    Int_t fNDFTbs;

  ClassDef(STPSAFastFit, 1)
};

#endif
