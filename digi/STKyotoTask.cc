#include "STKyotoTask.hh"
#include "STFairMCEventHeader.hh"

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
  if(fUseKatana)
    fLogger->Info(MESSAGE_ORIGIN, "Remember you must enable STDetector::SaveParentID() for Katana simulation to work.");

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
  bool reject_by_katana = false;
  int Z_max = 0;
  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    double posx = fMCPoint->GetX()*10; // to mm
    double posy = fMCPoint->GetY()*10; // to mm
    double posz = fMCPoint->GetZ()*10; // to mm

    const double katana_z_min = 1700; // any hit beyond z = 1700 mm must come from Katana
    if(fabs(posx) > fEnclosureWidth/2. && posz < katana_z_min)
      kyoto_trigged_id.insert(fMCPoint -> GetDetectorID());

    if(posz > katana_z_min && fUseKatana)
    {
      int pdg = fMCPoint -> GetPDG();
      int Z = (pdg%10000000)/10000;
      if(pdg > 10000000 && Z >= 20)
      {
        reject_by_katana = true;
        if(Z > Z_max) Z_max = Z;
      }
    }
  }

  int num_trigged = kyoto_trigged_id.size();
  bool reject = (num_trigged < 4 || reject_by_katana)? true : false;
 
  fEventID = fFairMCEventHeader -> GetEventID();
  fLogger->Info(MESSAGE_ORIGIN, 
            Form("Event #%d : Kyoto multiplicity %d. Katana max Z %d. %s event.",
                 fEventID, num_trigged, Z_max, (reject)? "Reject" : "Accept"));

  if(reject) fEventHeader -> SetIsEmptyEvent();
  // if no particles passes through kyoto array, it doesn't even satisfy minimum bias condition
  // will be designated status 10
  if(num_trigged == 0) fEventHeader -> SetStatus(10);
  //FairRunAna::Instance() -> MarkFill(!reject);
  return;
}

void STKyotoTask::SetVerbose(Bool_t value) { fVerbose = value; }

ClassImp(STKyotoTask);
