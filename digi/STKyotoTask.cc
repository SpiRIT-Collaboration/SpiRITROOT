#include "STKyotoTask.hh"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>

// Root class headers
#include "TLorentzVector.h"
#include "TString.h"
#include "TRandom.h"
#include "TError.h"

STKyotoTask::STKyotoTask()
:FairTask("STKyotoTask"),
 fEventID(0)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STKyotoTask");
}

STKyotoTask::~STKyotoTask()
{ fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STKyotoTask"); }

void 
STKyotoTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STKyotoTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STKyotoTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STKyotoTask");

  FairRootManager* ioman = FairRootManager::Instance();

  fMCPointArray = (TClonesArray*) ioman->GetObject("STMCPoint");
  fFairMCEventHeader = (FairMCEventHeader*) ioman->GetObject("MCEventHeader.");

  fEventHeader = new STEventHeader;
  ioman -> Register("STEventHeader", "SpiRIT", fEventHeader, true);

  fEnclosureWidth = 1477.4;//fPar->GetPadPlaneX();

  return kSUCCESS;
}


void 
STKyotoTask::Exec(Option_t* option)
{
  fEventHeader -> Clear();

  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STKyotoTask");
  Int_t nMCPoints = fMCPointArray->GetEntries();

  std::unordered_set<int> kyoto_trigged_id;
  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    double posx = fMCPoint->GetX()*10; // to mm
    double posy = fMCPoint->GetY()*10; // to mm
    double posz = fMCPoint->GetZ()*10; // to mm

    if(fabs(posx) > fEnclosureWidth/2.)
      kyoto_trigged_id.insert(fMCPoint -> GetDetectorID());
      
  }

  int num_trigged = kyoto_trigged_id.size();
  bool reject = (num_trigged >= 4)? false : true;

  fEventID = fFairMCEventHeader -> GetEventID();
  fLogger->Info(MESSAGE_ORIGIN, 
            Form("Event #%d : Kyoto multiplicity %d. %s event.",
                 fEventID, num_trigged, (reject)? "Reject" : "Accept"));

  if(reject) fEventHeader -> SetIsEmptyEvent();
  //FairRunAna::Instance() -> MarkFill(!reject);
  return;
}

void STKyotoTask::SetVerbose(Bool_t value) { fVerbose = value; }

ClassImp(STKyotoTask);
