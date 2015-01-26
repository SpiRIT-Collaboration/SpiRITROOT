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

#pragma once

// SpiRITROOT classes
#include "STPSA.hh"

// ROOT classes
#include "TSpectrum.h"

class STPSALayer : public STPSA
{
  public:
    STPSALayer();
    ~STPSALayer();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  private:
    void Reset();
    void PreAnalyze(STRawEvent *rawEvent);
    void DeletePeakInfo(STRawEvent *rawEvent, Short_t row, Short_t layer, Short_t peakNum);
    void LSLFit(Int_t numPoints, Double_t *x, Double_t *y, Double_t &constant, Double_t &slope);

    TSpectrum *fPeakFinder;   ///< TSpectrum object

    Int_t fNumSidePads;       ///< The number of pads to average side of the pad having the highest peak
    Short_t **fPadID;         ///< Pad ID with row and layer
    Short_t **fNumPeaks;      ///< The number of peaks in the fired pad

    Short_t fPeakStorageSize; ///< Maximum number of peaks in a pad
    Short_t ***fPeakTbs;      ///< Peak positions in amplitude increasing order
    Short_t ***fPeakValues;   ///< Peak amplitude in increasing order

    Short_t fMinPoints;       ///< Minimum points of slope for determing the hit time
    Short_t fPercPeakMin;     ///< Minimum percentage of peak for determinig the hit time
    Short_t fPercPeakMax;     ///< Maximum percentage of peak for determinig the hit time

  ClassDef(STPSALayer, 1)
};
