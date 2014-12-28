//---------------------------------------------------------------------
// Description:
//      Pad response task class source
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// ST header
#include "STElectronicsTask.hh"

// C/C++ class headers
#include <iostream>

using namespace std;

// ---- Default constructor -------------------------------------------
STElectronicsTask::STElectronicsTask()
  :FairTask("STElectronicsTask")
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STElectronicsTask");
}

// ---- Destructor ----------------------------------------------------
STElectronicsTask::~STElectronicsTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STElectronicsTask");
}

// ----  Initialisation  ----------------------------------------------
void STElectronicsTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STElectronicsTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

// ---- Init ----------------------------------------------------------
InitStatus STElectronicsTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STElectronicsTask");

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  fPPEventArray = (TClonesArray*) ioman->GetObject("STPPEvent");
  fRawEventArray = new TClonesArray("STRawEvent"); 
  ioman->Register("STRawEvent", "ST", fRawEventArray, kTRUE);

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus STElectronicsTask::ReInit()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STElectronicsTask");
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void STElectronicsTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STElectronicsTask");
}

// ---- Finish --------------------------------------------------------
void STElectronicsTask::Finish()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Finish of STElectronicsTask");
}

ClassImp(STElectronicsTask)
