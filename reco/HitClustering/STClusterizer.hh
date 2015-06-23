/**
 * @brief Virtual Clusterizer Class
 * @author JungWoo Lee (Korea Univ.)
 */

#pragma once

// SpiRITROOT classes
#include "STEvent.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "STDigiPar.hh"

// FairRoot classes
#include "FairRootManager.h"
#include "FairLogger.h"

// STL
#include <vector>

// ROOT classes
#include "TClonesArray.h"

class STClusterizer
{
  public:
    STClusterizer();
    virtual ~STClusterizer();

    virtual void Analyze(STEvent* eventH, STEvent* eventHC) = 0;

  protected:
    FairLogger *fLogger;      //!< logger
    STDigiPar *fPar;          //!< parameter

    Int_t fPadPlaneX;         //!< pad plane size x in mm
    Int_t fPadSizeX;          //!< pad size x in mm
    Int_t fPadSizeZ;          //!< pad size y in mm
    Int_t fPadRows;           //!< number of total pad rows
    Int_t fPadLayers;         //!< number of total pad layers

    Int_t fNumTbs;            //!< the number of time buckets used in taking data
    Int_t fTBTime;            //!< time duration of a time bucket in ns
    Double_t fDriftVelocity;  //!< drift velocity of electron in cm/us
    Double_t fMaxDriftLength; //!< maximum drift length in mm

  ClassDef(STClusterizer, 1)
};
