/**
 * @brief Pulse shape analysis with the maximum gradient
 * @author Genie Jhang (NSCL)
 */

#ifndef STPSAGRAD
#define STPSAGRAD

// SpiRITROOT classes
#include "STPSA.hh"
#include "STGlobal.hh"

// ROOT classes
#include "TSpectrum.h"
#include "TClonesArray.h"

// STL
#include <mutex>
#include <condition_variable>

class STPSAGrad : public STPSA
{
  public:
    STPSAGrad();
    ~STPSAGrad();

    void Init();

    void Analyze(STRawEvent *rawEvent, STEvent *event);
    void Analyze(STRawEvent *rawEvent, TClonesArray *hitArray);
    void PadAnalyzer(TClonesArray *hitArray);

    void FindHits(STPad *pad, TClonesArray *hitArray, Int_t &hitNum);

    Bool_t FindPeak(Double_t *adc, Int_t &tbCurrent, Int_t &tbStart);
    void FindMaxGrad(Double_t *adc, Int_t tbCurrent, Double_t &grad);

  private:
    TClonesArray **fThreadHitArray; ///< TClonesArray object for thread

    Int_t fPadIndex;
    Int_t fNumPads;
    std::vector<STPad> *fPadArray;

    std::mutex fMutex;

    Int_t fNumAscending;
    Int_t fTbStartCut;

  ClassDef(STPSAGrad, 1)
};

#endif
