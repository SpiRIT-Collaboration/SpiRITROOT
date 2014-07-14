//---------------------------------------------------------------------
// Description:
//      Pad Response task class source
//
//      Make pad signals
//
//      Input  : STAvalanche
//      Output : STPadSignal
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STPadResponseTask.hh"

// Fair & Collaborating class headers
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"


// SPiRIT-TPC headers
#include "STAvalanche.hh"
#include "STPadResponse.hh"

// ROOT headers

// C/C++ headers
#include "cmath"
#include "iostream"

using std::cout;
using std::endl;


ClassImp(STPadResponseTask)

STPadResponseTask::STPadResponseTask()
: FairTask("SPiRIT PadResponse"),
  fIsPersistent(kFALSE)
{
  fAvalancheBranchName = "STAvalanche";
}

STPadResponseTask::~STPadResponseTask()
{
}

InitStatus
STPadResponseTask::Init()
{
  // Get ROOT Manager
  FairRootManager* ioman = FairRootManager::Instance();

  if(!ioman) {
    Error("STPadResponse::Init", "RootManager not instantiated!");
    return kERROR;
  }

  // Get input collection
  fAvalancheArray = (TClonesArray *) ioman -> GetObject(fAvalancheBranchName);
  if(!fAvalancheArray) {
    Error("STPadResponseTask::Init", "Point-array not found!");
    return kERROR;
  }

  // Create and register output array
  fPadSignal = new TClonesArray("STPadSignal");
  ioman -> Register("STPadResponse", "ST", fPadSignal, fIsPersistent);

  fGas = fPar -> GetGas();

  fPadPlane = fPar -> GetPadPlane();

  return kSUCCESS;
}

void
STPadResponseTask::SetParContainers()
{
  cout << "STPadResponseTask::SetParContainers" << endl; //cout.flush()

  FairRun* run = FairRun::Instance();
  if(!run) Fatal("SetParContainers","No analysis run");

  FairRuntimeDb* db = run -> GetRuntimeDb();
  if(!db) Fatal("SetParContainers", "no runtime database");

  fPar = (STDigiPar*) db -> getContainer("STDigiPar");
  if(!fPar) Fatal("SetParContainers", "STDigiPar not found");
}

void
STPadResponseTask::Exec(Option_t *opt)
{
  if(fPadSignalArray==0) Fatal("STPadResponse::Exec)","No Pad Signal Array");
  PadSignalArray -> Delete();

  Int_t nAvalanche = fAvalancheArray -> GetEntriesFast();
  for(Int_t iAvalanche=0; iAvalanche<nElectron; iAvalanche++)
  {
    STAvalanche* avalanche = (STAvalanche*) fAvalancheArray -> At(iAvalanche);
    Int_t nSignal = fPadSignalArray -> GetEntries();

    gain = gain; // this should be fixed once we get the garfield data.
    
    STPadSignal* signal 
      = new ((*fPadSignalArray)[nSignal]) STPadSignal();
    avalanche -> SetIndex(nSignal);
  }

  cout << "STPadResponseTask:: " << fPadSignalArray -> GetEntriesFast() 
       << " avalanche created" << endl; 

  return;
}
