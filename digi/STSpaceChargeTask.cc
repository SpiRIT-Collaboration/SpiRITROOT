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
 fIsDrift(kTRUE),
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
  fDispMCPointArray = new TClonesArray("STMCPoint");
  ioman->Register("DispMCPoint","ST",fDispMCPointArray,fIsPersistence);

  const double TPCHeight = 506.1;
  this -> SetTPCSize(fPar->GetPadPlaneX(), fPar->GetPadPlaneZ(), TPCHeight);

  if(fIsDrift) this -> CalculateEDrift(fPar->GetDriftVelocity());
  else fLogger->Info(MESSAGE_ORIGIN, "Space Chrage displacement is disabled");
  return kSUCCESS;
}


void 
STSpaceChargeTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STSpaceChargeTask");
  if(!fDispMCPointArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No Drifted MC Point Array!");
  fDispMCPointArray -> Delete();
  Int_t nMCPoints = fMCPointArray->GetEntries();
  /**
   * NOTE! that fMCPoint has unit of [cm] for length scale,
   * [GeV] for energy and [ns] for time.
   */
  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    Int_t index = fDispMCPointArray->GetEntriesFast();
    STMCPoint *drifted_mc_point
      = new ((*fDispMCPointArray)[index])
        STMCPoint(*fMCPoint);
    double posx, posy, posz;
    if(fIsDrift)
    {
      this -> DisplaceElectrons(fMCPoint->GetX(), fMCPoint->GetY(), fMCPoint->GetZ(), posx, posy, posz);
      drifted_mc_point -> SetXYZ(posx, posy, posz);
    }
  }

  if(fIsDrift)
    fLogger->Info(MESSAGE_ORIGIN, 
              Form("Event #%d : MC points (%d) found. They are dispaced due to space charge",
                   fEventID++, nMCPoints));

  return;
}

void STSpaceChargeTask::SetPersistence(Bool_t value) { fIsPersistence = value; }
void STSpaceChargeTask::SetVerbose(Bool_t value) { fVerbose = value; }
void STSpaceChargeTask::SetElectronDrift(Bool_t value) { fIsDrift = value; }

ClassImp(STSpaceChargeTask);
