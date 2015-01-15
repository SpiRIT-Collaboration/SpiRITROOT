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
#include "STPadResponseTask.hh"
#include "STProcessManager.hh"

// C/C++ class headers
#include <iostream>

using namespace std;

// ---- Default constructor -------------------------------------------
STPadResponseTask::STPadResponseTask()
  :FairTask("STPadResponseTask")
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STPadResponseTask");
}

// ---- Destructor ----------------------------------------------------
STPadResponseTask::~STPadResponseTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STPadResponseTask");
}

// ----  Initialisation  ----------------------------------------------
void STPadResponseTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STPadResponseTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

// ---- Init ----------------------------------------------------------
InitStatus STPadResponseTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STPadResponseTask");

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  fDigitizedElectronArray = (TClonesArray*) ioman->GetObject("STDigitizedElectron");
  fRawEventArray = new TClonesArray("STRawEvent"); 
  ioman->Register("PPEvent", "ST", fRawEventArray, kTRUE);

  fPadResponse  = new STPadResponse();

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus STPadResponseTask::ReInit()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STPadResponseTask");
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void STPadResponseTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STPadResponseTask");

  if(!fRawEventArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No RawEventArray!");

  fRawEvent = new ((*fRawEventArray)[0]) STRawEvent();

  fPadResponse -> Init();

  Int_t nElectrons = fDigitizedElectronArray -> GetEntries();
  fLogger->Info(MESSAGE_ORIGIN, Form("There are %d digitized electrons.",nElectrons));

  STProcessManager fProcess("PadReponse", nElectrons);
  for(Int_t iElectron=0; iElectron<nElectrons; iElectron++)
  {
    fProcess.PrintOut(iElectron);
    fDigiElectron = (STDigitizedElectron*) fDigitizedElectronArray -> At(iElectron);

    Double_t xEl = fDigiElectron -> GetX();
    Double_t tEl = fDigiElectron -> GetTime();
    Int_t    zWi = fDigiElectron -> GetZWire();
    Int_t   gain = fDigiElectron -> GetGain();

    fPadResponse -> FillPad(gain, xEl, tEl, zWi);
  }
  fProcess.End();

  fPadResponse -> CloneRawEvent(fRawEvent);

  Int_t nPads = fRawEvent -> GetNumPads();
  fLogger->Info(MESSAGE_ORIGIN, 
                Form("Pad plane event created. There are %d active pads.",nPads));

  //delete fRawEvent;

  return;
}

// ---- Finish --------------------------------------------------------
void STPadResponseTask::Finish()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Finish of STPadResponseTask");
}

ClassImp(STPadResponseTask);
