/**
 * @brief Find all peaks and make each of them into hits.
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STPSAALL
#define STPSAALL

// SpiRITROOT classes
#include "STPSA.hh"

// ROOT classes
#include "TSpectrum.h"
#include "TClonesArray.h"

// STL
#include <mutex>
#include <condition_variable>

class STPSAAll : public STPSA
{
  public:
    STPSAAll();

    void Analyze(STRawEvent *rawEvent, STEvent *event);
    void PadAnalyzer(TClonesArray *hitArray);

  private:
    TSpectrum *fPeakFinder;  /// TSpectrum object
    TClonesArray **fThreadHitArray; /// TClonesArray object

    Bool_t fPadReady;
    Bool_t fPadTaken;
    Bool_t fEnd;

    std::mutex fMutex;
    std::condition_variable fCondVariable;

    STPad *fPad;

  ClassDef(STPSAAll, 2)
};

#endif
