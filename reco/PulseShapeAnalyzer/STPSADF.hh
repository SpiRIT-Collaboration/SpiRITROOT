/**
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STPSADF_HH
#define STPSADF_HH

// SpiRITROOT classes
#include "STPSA.hh"

class STPSADF : public STPSA
{
  public:
    STPSADF();
    ~STPSADF();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  private:
    Double_t fMaxFitPoints;
    Double_t fMinFitPoints;
    vector<Int_t> fBinCandidates;

  ClassDef(STPSADF, 1)
};

#endif
