//---------------------------------------------------------------------
// Description:
//      Avalanche task class source
//
//      Make avalanch electrons drifted through wire plane.
//
//      Input  : STDritedElectron
//      Output : STAvalanche
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STAvalancheTask.hh"

// Fair & Collaborating class headers
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"


// SPiRIT-TPC headers
#include "STDritedElectron.hh"
#include "STAvalanche.hh"

// ROOT headers
#include "TRandom.h"

// C/C++ headers
#include "cmath"
#include "iostream"

using std::cout;
using std::endl;


ClassImp(STAvalancheTask)

STAvalancheTask::STAvalancheTask()
: FairTask("SPiRIT Avalanche"),
  fIsPersistent(kFALSE)
{
  fDriftedElectronBranchName = "STDritedElectron";
}

STAvalancheTask::~STAvalancheTask()
{
}

InitStatus
STAvalancheTask::Init()
{
  // Get ROOT Manager
  FairRootManager* ioman = FairRootManager::Instance();

  if(!ioman) {
    Error("STAvalanchekTask::Init", "RootManager not instantiated!");
    return kERROR;
  }

  // Get input collection
  fPrimaryClusterArray = (TClonesArray *) ioman -> GetObject(fDriftedElectronBranchName);
  if(!fPrimaryClusterArray) {
    Error("STClusterizerTask::Init", "Point-array not found!");
    return kERROR;
  }

  // Create and register output array
  fDriftedElectronArray = new TClonesArray("STDritedElectron");
  ioman -> Register("STDritedElectron", "ST", fDriftedElectronArray, fIsPersistent);

  fGas = fPar -> GetGas();

  return kSUCCESS;
}

void
STAvalancheTask::SetParContainers()
{
  cout << "STAvalancheTask::SetParContainers" << endl; //cout.flush()

  FairRun* run = FairRun::Instance();
  if(!run) Fatal("SetParContainers","No analysis run");

  FairRuntimeDb* db = run -> GetRuntimeDb();
  if(!db) Fatal("SetParContainers", "no runtime database");

  fPar = (STDigiPar*) db -> getContainer("STDigiPar");
  if(!fPar) Fatal("SetParContainers", "STDigiPar not found");
}

void
STAvalancheTask::Exec(Option_t *opt)
{
  if(fDriftedElectronArray==0) Fatal("STDritedElectron::Exec)","No Avalanche Array");
  fDriftedElectronArray -> Delete();


  cout << "STAvalancheTask:: " << fDriftedElectronArray -> GetEntriesFast() 
       << " avalanche created" << endl; 

  return;
}
