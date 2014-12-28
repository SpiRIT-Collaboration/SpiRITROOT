//---------------------------------------------------------------------
// Description:
//      Drift electron task class source
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
#include "STDriftTask.hh"
#include "STProcessManager.hh"

// C/C++ class headers
#include <cmath>
#include <iostream>

// Root class headers
#include "TLorentzVector.h"
#include "TString.h"

using namespace std;

// ---- Default constructor -------------------------------------------
STDriftTask::STDriftTask()
  :FairTask("STDriftTask")
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STDriftTask");
}

// ---- Destructor ----------------------------------------------------
STDriftTask::~STDriftTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STDriftTask");
}

// ----  Initialisation  ----------------------------------------------
void 
STDriftTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STDriftTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

// ---- Init ----------------------------------------------------------
InitStatus 
STDriftTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STDriftTask");

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  fMCPointArray = (TClonesArray*) ioman->GetObject("STMCPoint");
  fDigitizedElectronArray = new TClonesArray("STDigitizedElectron");
  ioman->Register("STDigitizedElectron","ST",fDigitizedElectronArray,kTRUE);

  fGas = fPar->GetGas();
  fEIonize = (fGas->GetEIonize())*1.E6; // [MeV] to [eV]

  fDriftElectron = new STDriftElectron();
  fWireResponse  = new STWireResponse();

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus STDriftTask::ReInit()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STDriftTask");
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void STDriftTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STDriftTask");

  if(!fDigitizedElectronArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No DigitizedElectronArray!");
  fDigitizedElectronArray -> Delete();

  Int_t nMCPoints = fMCPointArray->GetEntries();
  if(nMCPoints<20){
    fLogger->Warning(MESSAGE_ORIGIN, "Not enough hits for digitization!");
    return;
  }
  fLogger->Info(MESSAGE_ORIGIN, Form("There are %d MC points.",nMCPoints));

  TLorentzVector V4MC;
  TLorentzVector V4Drift;
  Double_t       zWire;
  Double_t       gain;

  STProcessManager fProcess(TString("Drifiting"), nMCPoints);
  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fProcess.PrintOut(iPoint);
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    Double_t eLoss = (fMCPoint->GetEnergyLoss())*1.E9; // [GeV] to [eV]
    if(eLoss<0) continue;

    V4MC.SetXYZT(fMCPoint->GetX(),fMCPoint->GetY(),fMCPoint->GetZ(),fMCPoint->GetTime());
    fDriftElectron->SetMCHit(V4MC);
    
    Int_t nElectrons = (Int_t)floor(fabs(eLoss/fEIonize));
    for(Int_t iElectron=0; iElectron<nElectrons; iElectron++) {
      V4Drift = fDriftElectron->Drift();
      zWire   = fWireResponse->FindZWire(V4Drift.Z());

      Int_t index = fDigitizedElectronArray->GetEntriesFast();
      STDigitizedElectron* electron
        = new ((*fDigitizedElectronArray)[index])
          STDigitizedElectron(V4Drift.X(), V4Drift.Z(), zWire, V4Drift.T(),1);
      electron -> SetIndex(index);
    }
  }
  fProcess.End();

  Int_t nDigiElectrons = fDigitizedElectronArray->GetEntriesFast();
  fLogger->Info(MESSAGE_ORIGIN, Form("%d digitized electrons created.",nDigiElectrons));

  return;
}

// ---- Finish --------------------------------------------------------
void STDriftTask::Finish()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Finish of STDriftTask");
}

ClassImp(STDriftTask);
