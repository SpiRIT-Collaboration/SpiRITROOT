//-----------------------------------------------------------
// Description:
//   Simple version 2 of analyzing pulse shape of raw signal.
//   This version uses TSpectrum class in ROOT to find
//   peaks in pads.
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef STPSASIMPLE2_H
#define STPSASIMPLE2_H

// SpiRITROOT classes
#include "STRawEvent.hh"
#include "STPad.hh"
#include "STEvent.hh"
#include "STHit.hh"
#include "STDigiPar.hh"

// FairRoot classes
#include "FairRootManager.h"
#include "FairLogger.h"

// STL
#include <vector>

// ROOT classes
#include "TClonesArray.h"
#include "TSpectrum.h"

class STPSASimple2
{
  public:
    STPSASimple2();
    ~STPSASimple2();

    //! Setting threshold
    void SetThreshold(Int_t threshold);

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  private:
    FairLogger *fLogger;     //!< logger pointer
    STDigiPar *fPar;         //!< parameter container

    Int_t fPadPlaneX;        //!< pad plane size x in mm
    Int_t fPadSizeX;         //!< pad size x in mm
    Int_t fPadSizeZ;         //!< pad size y in mm

    Int_t fNumTbs;           //!< the number of time buckets used in taking data
    Int_t fTBTime;           //!< time duration of a time bucket in ns
    Double_t fDriftVelocity; //!< drift velocity of electron in cm/us

    Int_t fThreshold;        //!< threshold of ADC value

    TSpectrum *fPeakFinder;  /// TSpectrum object

    //!< Calculate x position in mm. This returns the center position of given pad row.
    Double_t CalculateX(Int_t row);
    //!< Calculate y position in mm using the peak index.
    Double_t CalculateY(Int_t peakIdx);
    //!< Calculate z position in mm. This returns the center position of given pad layer.
    Double_t CalculateZ(Int_t layer);

  ClassDef(STPSASimple2, 1)
};

#endif
