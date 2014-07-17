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

// SpiRITROOT classes
#include "STHitClusteringSimple.hh"
#include "STHitCluster.hh"

STHitClusteringSimple::STHitClusteringSimple()
{
  fLogger = FairLogger::FairLogger();
}

STHitClusteringSimple::~STHitClusteringSimple()
{
}

Bool_t
STHitClusteringSimple::FindCluster(STEvent *eventHArray, STEvent *eventHCArray)
{
}
