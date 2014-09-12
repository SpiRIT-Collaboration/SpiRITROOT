//---------------------------------------------------------------------
// Description:
//      Avalanche task class source
//
//      Make avalanch electrons drifted through wire plane.
//
//      Input  : STDriftedElectron
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
#include "STDriftedElectron.hh"
#include "STAvalanche.hh"

// ROOT headers
#include "TRandom.h"

// C/C++ headers
#include "cmath"
#include "iostream"

using std::cout;
using std::endl;


ClassImp(STAvalancheTask);

STAvalancheTask::STAvalancheTask()
: FairTask("SPiRIT Avalanche"),
  fIsPersistent(kFALSE),
  fTestMode(kFALSE)
{
  fDriftedElectronBranchName = "STDriftedElectron";
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
  fDriftedElectronArray = (TClonesArray *) ioman -> GetObject(fDriftedElectronBranchName);
  if(!fDriftedElectronArray) {
    Error("STAvalancheTask::Init", "Point-array not found!");
    return kERROR;
  }

  // Create and register output array
  fAvalancheArray = new TClonesArray("STAvalanche");
  ioman -> Register("STAvalanche", "ST", fAvalancheArray, fIsPersistent);

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
  if(fAvalancheArray==0) Fatal("STAvalanche::Exec)","No Avalanche Array");
  fAvalancheArray -> Delete();

  Int_t gain = fGas -> GetGain();
  if(fTestMode) cout << "gain : " << gain << endl;

  Int_t nElectron = fDriftedElectronArray -> GetEntriesFast();
  for(Int_t iElectron=0; iElectron<nElectron; iElectron++)
  {
    STDriftedElectron* electron = (STDriftedElectron*) fDriftedElectronArray -> At(iElectron);
    Int_t nAvalanche = fAvalancheArray -> GetEntries();

    STAvalanche* avalanche 
      = new ((*fAvalancheArray)[nAvalanche]) STAvalanche(electron -> GetX(),
                                                         electron -> GetZ(),
                                                         electron -> GetTime(),
                                                         gain);
    avalanche -> SetIndex(nAvalanche);
  }

  cout << "STAvalancheTask:: " << fAvalancheArray -> GetEntriesFast() 
       << " avalanche created" << endl; 

  return;
}
