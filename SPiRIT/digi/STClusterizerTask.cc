//---------------------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      STClusterizer reads in MCPoints and produces primary electrons
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//
// Rewritten for SPiRIT-TPC:
//      Genie Jhang     Korea Univ.
//
//----------------------------------------------------------------------

#include "STClusterizerTask.hh"

ClassImp(STClusterizerTask)

STClusterizerTask()::STClusterizerTask()
:FairTask("SPiRIT Clusterizer")
{
  fMCPointBranchName = "STPoint";
}

STClusterizerTask()::~STClusterizerTask()
{
}

InitStatus
STClusterizerTask::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman = FairRootManager::Instance();

  if(!ioman) {
    Error("STClusterizerTask::Init", "RootManager not instantiated!");
    return kERROR;
  }
  
  // Get input collection
  fMCPointArray = (TClonesArray *) ioman -> GetObject(fMCPointBranchName);
  
  if(!fMCPointArray) {
    Error("STClusterizerTask::Init", "Point-array not found!");
    return kERROR;
  }
  
  // Create and register output array
  fPrimaryClusterArray = new TClonesArray("STPrimaryCluster"); 
  ioman -> Register("STPrimaryCluster", "ST", fPrimaryClusterArray, fIsPersistent);

  fGas = fPar -> GetGas();

  return kSUCCESS;
}

void
STClusterizerTask::Exec(Option_t *opt)
{
}
