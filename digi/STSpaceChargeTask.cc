#include "STSpaceChargeTask.hh"
#include "STProcessManager.hh"

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

STSpaceChargeTask::STSpaceChargeTask()
:FairTask("STSpaceChargeTask"),
 fEventID(0),
 fIsPersistence(kFALSE)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STSpaceChargeTask");
}

STSpaceChargeTask::~STSpaceChargeTask()
{ fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STSpaceChargeTask"); }

void 
STSpaceChargeTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STSpaceChargeTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STSpaceChargeTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STSpaceChargeTask");

  FairRootManager* ioman = FairRootManager::Instance();

  fMCPointArray = (TClonesArray*) ioman->GetObject("STMCPoint");
  fFairMCEventHeader = (FairMCEventHeader*) ioman->GetObject("MCEventHeader.");

  const double TPCHeight = 506.1;
  this -> SetTPCSize(fPar->GetPadPlaneX(), fPar->GetPadPlaneZ(), TPCHeight);

  const double EField = 127.4; // V/cm
  const double BField = 0.5; // T
  this -> InferDriftParameters(fPar->GetDriftVelocity(), EField, BField);
  this -> CalculateEDrift(fPar->GetDriftVelocity());
  return kSUCCESS;
}


void 
STSpaceChargeTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STSpaceChargeTask");
  Int_t nMCPoints = fMCPointArray->GetEntries();
  /**
   * NOTE! that fMCPoint has unit of [cm] for length scale,
   * [GeV] for energy and [ns] for time.
   */
  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    double posx, posy, posz;
    this -> DisplaceElectrons(fMCPoint->GetX(), fMCPoint->GetY(), fMCPoint->GetZ(), posx, posy, posz);
    fMCPoint -> SetXYZ(posx, posy, posz);
  }

  fEventID = fFairMCEventHeader -> GetEventID();
  fLogger->Info(MESSAGE_ORIGIN, 
            Form("Event #%d : MC points (%d) found. They are dispaced due to space charge",
                 fEventID, nMCPoints));

  return;
}

void STSpaceChargeTask::SetPersistence(Bool_t value) { fIsPersistence = value; }
void STSpaceChargeTask::SetVerbose(Bool_t value) { fVerbose = value; }

ClassImp(STSpaceChargeTask);
