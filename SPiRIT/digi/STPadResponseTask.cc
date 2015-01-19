/**
 * @brief Process drifting of electron from created position to anode wire
 * plane. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail See header file or detail.
 */

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// ST header
#include "STPadResponseTask.hh"
#include "STProcessManager.hh"

// C/C++ class headers
#include <iostream>

using namespace std;

STPadResponseTask::STPadResponseTask()
:FairTask("STPadResponseTask"),
 fEventID(0)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STPadResponseTask");
}

STPadResponseTask::~STPadResponseTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STPadResponseTask");
}

void 
STPadResponseTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STPadResponseTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STPadResponseTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STPadResponseTask");

  FairRootManager* ioman = FairRootManager::Instance();

  fElectronArray = (TClonesArray*) ioman->GetObject("STDriftedElectron");
  fRawEventArray = new TClonesArray("STRawEvent"); 
  ioman->Register("PPEvent", "ST", fRawEventArray, fInputPersistance);

  fPadResponse  = new STPadResponse();

  return kSUCCESS;

}

void 
STPadResponseTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STPadResponseTask");

  if(!fRawEventArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No RawEventArray!");

  fRawEvent = new ((*fRawEventArray)[0]) STRawEvent();

  fPadResponse -> Init();

  Int_t nElectrons = fElectronArray -> GetEntries();
  for(Int_t iElectron=0; iElectron<nElectrons; iElectron++)
  {
    fElectron = (STDriftedElectron*) fElectronArray -> At(iElectron);

    Double_t xEl = fElectron -> GetX();
    Double_t tEl = fElectron -> GetTime();
    Int_t    zWi = fElectron -> GetZWire();
    Int_t   gain = fElectron -> GetGain();

    fPadResponse -> FillPad(gain, xEl, tEl, zWi);
  }

  fPadResponse -> CloneRawEvent(fRawEvent);

  Int_t nPads = fRawEvent -> GetNumPads();
  fLogger->Info(MESSAGE_ORIGIN, 
                Form("Event #%d : Active pads (%d) created.",
                     fEventID++, nPads));

  return;
}

ClassImp(STPadResponseTask);
