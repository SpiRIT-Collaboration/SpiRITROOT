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

class STPSAAll : public STPSA
{
  public:
    STPSAAll();
    ~STPSAAll();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  private:
    TSpectrum *fPeakFinder;  /// TSpectrum object

  ClassDef(STPSAAll, 2)
};

#endif
