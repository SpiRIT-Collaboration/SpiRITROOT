//-----------------------------------------------------------
// Description:
//   Simple version of clustering hits
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef STHITCLUSTERINGSIMPLE_H
#define STHITCLUSTERINGSIMPLE_H

// SpiRITROOT clasees
#include "STEvent.hh"

// FairROOT classes
#include "FairLogger.h"

// ROOT classes
#include "Rtypes.h"

class STHitClusteringSimple
{
  public:
    STHitClusteringSimple();
    ~STHitClusteringSimple();

    Bool_t FindCluster(STEvent *eventHArray, STEvent *eventHCArray);

  private:
    FairLogger *fLogger;

    Int_t fYSliceDivider;

  ClassDef(STHitClusteringSimple, 1)
};

#endif
