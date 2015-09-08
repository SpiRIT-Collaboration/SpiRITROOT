//-----------------------------------------------------------
// Description:
//   This version uses TSpectrum class in ROOT to find
//   peaks in pads and in one layer averages certain number
//   pads around the pad having the highest peak.
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef STPSALAYER
#define STPSALAYER

// SpiRITROOT classes
#include "STPSA.hh"

// ROOT classes
#include "TSpectrum.h"

// STL
#include <vector>

class STPeak
{
  public:
    Int_t index;
    Int_t tb;
    Double_t value;

    void Reset() { index = -1; tb = -1; value = -1; } 

    STPeak &operator=(STPeak right) { index = right.index; tb = right.tb; value = right.value; return *this; }
};

class STPSALayer : public STPSA
{
  public:
    STPSALayer();
    ~STPSALayer();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  private:
    void Reset();
    Int_t GetArrayIdx(Int_t row, Int_t layer);
    Int_t GetUnusedPadIdx();
    void PreAnalyze();
    void DeletePeakInfo(Int_t row, Int_t layer, Int_t peakNum);

    TSpectrum *fPeakFinder;                    ///< TSpectrum object

    std::vector<STPad> *fPadArray;             ///< Pad array pointer in STRawEvent

    Int_t fNumFiredPads;                       ///< The number of total fired pads
    Int_t *fPadIdxArray;                       ///< Array of pad index in fPadArray
    Int_t fArrayIdx;                           ///< Row index to point unused pad

    Int_t fNumSidePads;                        ///< The number of pads to average side of the pad having the highest peak
    Int_t fNumSideTbs;                         ///< The number of tbs to search peak near the maximum peak
    Int_t **fNumPeaks;                         ///< The number of peaks in the fired pad

    Int_t fPeakStorageSize;                    ///< Maximum number of peaks in a pad
    STPeak ***fPeaks;                          ///< Peak container pointer
    Bool_t fIgnoreLeft;
    STPeak fPrevLeftPeak;
    Bool_t fIgnoreRight;
    STPeak fPrevRightPeak;

    Int_t fMinPoints;                          ///< Minimum points of slope for determing the hit time
    Int_t fPercPeakMin;                        ///< Minimum percentage of peak for determinig the hit time
    Int_t fPercPeakMax;                        ///< Maximum percentage of peak for determinig the hit time

  ClassDef(STPSALayer, 1)
};

#endif
