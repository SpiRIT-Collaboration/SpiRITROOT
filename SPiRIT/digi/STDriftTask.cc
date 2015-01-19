/**
 * @brief Process drifting of electron from created position to anode wire
 * plane. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail See header file for detail.
 */

// This class & SPiRIT class headers
#include "STDriftTask.hh"
#include "STProcessManager.hh"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>

// Root class headers
#include "TLorentzVector.h"
#include "TString.h"
#include "TRandom.h"

using namespace std;

STDriftTask::STDriftTask()
:FairTask("STDriftTask"),
 fEventID(0)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STDriftTask");
}

STDriftTask::~STDriftTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STDriftTask");
}

void 
STDriftTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STDriftTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STDriftTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STDriftTask");

  FairRootManager* ioman = FairRootManager::Instance();

  fMCPointArray = (TClonesArray*) ioman->GetObject("STMCPoint");
  fElectronArray = new TClonesArray("STDriftedElectron");
  ioman->Register("STDriftedElectron","ST",fElectronArray,fInputPersistance);

  fGas = fPar->GetGas();
  fEIonize = (fGas->GetEIonize())*1.E6; // [MeV] to [eV]

  fDriftElectron = new STDriftElectron();
  fWireResponse  = new STWireResponse();

  return kSUCCESS;

}

void 
STDriftTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STDriftTask");

  if(!fElectronArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No DigitizedElectronArray!");
  fElectronArray -> Delete();

  Int_t nMCPoints = fMCPointArray->GetEntries();
  if(nMCPoints<20){
    fLogger->Warning(MESSAGE_ORIGIN, "Not enough hits for digitization!");
    return;
  }

  TLorentzVector v4MC;    // [mm]
  TLorentzVector v4Drift; // [mm]
  Int_t          zWire;   // [mm]
  Int_t          gain0 = fGas -> GetGain();

  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    Double_t eLoss = (fMCPoint->GetEnergyLoss())*1.E9; // [GeV] to [eV]
    if(eLoss<0) continue;

    v4MC.SetXYZT(fMCPoint->GetX()*10,
                 fMCPoint->GetY()*10,
                 fMCPoint->GetZ()*10,
                 fMCPoint->GetTime());
    fDriftElectron->SetMCHit(v4MC);
    
    Int_t nElectrons = (Int_t)floor(fabs(eLoss/fEIonize));
    for(Int_t iElectron=0; iElectron<nElectrons; iElectron++) {
      Int_t gain = gRandom -> Gaus(gain0,30);
      if(gain<=0) continue;
      v4Drift = fDriftElectron->Drift();
      zWire   = fWireResponse->FindZWire(v4Drift.Z());

      Int_t index = fElectronArray->GetEntriesFast();
      STDriftedElectron* electron
        = new ((*fElectronArray)[index])
          STDriftedElectron(v4Drift.X(), v4Drift.Z(), zWire, v4Drift.T(), gain);
      electron -> SetIndex(index);
    }
  }

  Int_t nDriftElectrons = fElectronArray->GetEntriesFast();

  fLogger->Info(MESSAGE_ORIGIN, 
                Form("Event #%d : MC points (%d) found. Drift electrons (%d) created.",
                     fEventID++, nMCPoints, nDriftElectrons));


  return;
}

ClassImp(STDriftTask);
