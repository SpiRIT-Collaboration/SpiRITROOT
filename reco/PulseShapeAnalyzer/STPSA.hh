//-----------------------------------------------------------
// Description:
//   Simple version of analyzing pulse shape of raw signal.
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef STPSASIMPLE_H
#define STPSASIMPLE_H

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

class STPSA
{
  public:
    STPSA();
    ~STPSA();

    //! Setting threshold
    void SetThreshold(Int_t threshold);

    virtual void Analyze(STRawEvent *rawEvent, STEvent *event) = 0;

  protected:
    FairLogger *fLogger;      ///< logger pointer
    STDigiPar *fPar;          ///< parameter container

    Int_t fPadPlaneX;         ///< pad plane size x in mm
    Int_t fPadSizeX;          ///< pad size x in mm
    Int_t fPadSizeZ;          ///< pad size y in mm
    Int_t fPadRows;           ///< number of total pad rows
    Int_t fPadLayers;         ///< number of total pad layers

    Int_t fNumTbs;            ///< the number of time buckets used in taking data
    Int_t fTBTime;            ///< time duration of a time bucket in ns
    Double_t fDriftVelocity;  ///< drift velocity of electron in cm/us
    Double_t fMaxDriftLength; ///< maximum drift length in mm

    Int_t fThreshold;         ///< threshold of ADC value

    Double_t CalculateX(Double_t row);      ///< Calculate x position in mm. This returns the center position of given pad row.
    Double_t CalculateY(Double_t peakIdx);  ///< Calculate y position in mm using the peak index.
    Double_t CalculateZ(Double_t layer);    ///< Calculate z position in mm. This returns the center position of given pad layer.

  ClassDef(STPSA, 2)
};

#endif
