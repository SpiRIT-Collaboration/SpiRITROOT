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
  ioman->Register("STRawEvent", "ST", fRawEventArray, kTRUE);

  fWireResponse  = new STWireResponse();
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

  InitializeRawEvent();
  fPadResponse -> SetRawEvent(fRawEvent);

  Int_t nElectrons = fDigitizedElectronArray -> GetEntries();
  fLogger->Info(MESSAGE_ORIGIN, Form("There are %d digitized electrons.",nElectrons));

  STProcessManager fProcess("PadReponse", nElectrons);
  for(Int_t iElectron=0; iElectron<nElectrons; iElectron++)
  {
    fProcess.PrintOut(iElectron);
    fDigiElectron = (STDigitizedElectron*) fDigitizedElectronArray -> At(iElectron);
    Double_t xEl = fDigiElectron -> GetX();
    Double_t tEl = fDigiElectron -> GetTime();
    Double_t zWi = fDigiElectron -> GetZWire();

    fPadResponse -> FillPad(xEl, tEl, zWi);
  }
  fProcess.End();
  fPadResponse -> WriteHistogram();

  fLogger->Info(MESSAGE_ORIGIN, "Raw event created.");

  new ((*fRawEventArray)[0]) STRawEvent(fRawEvent);
  delete fRawEvent;

  return;
}

// ---- Finish --------------------------------------------------------
void STPadResponseTask::Finish()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Finish of STPadResponseTask");
}

void
STPadResponseTask::InitializeRawEvent()
{
  nTBs = fPar -> GetNumTbs(); // number of time buckets

  fRawEvent = new STRawEvent();
  fRawEvent -> SetName("RawEvent");
  fRawEvent -> SetEventID(1);

  fMap = new STMap();
  fMap -> SetUAMap((fPar -> GetFile(0)).Data());
  fMap -> SetAGETMap((fPar -> GetFile(1)).Data());

  Int_t row, layer;

  for(Int_t iCoBo=0; iCoBo<12; iCoBo++){
    for(Int_t iAsAd=0; iAsAd<4; iAsAd++){
      for(Int_t iAGET=0; iAGET<4; iAGET++){
        for(Int_t iCh=0; iCh<68; iCh++){

          Bool_t isActive = fMap -> GetRowNLayer(iCoBo, iAsAd, iAGET, iCh, row, layer);
          if(!isActive) continue;

          STPad* pad = new STPad(row,layer);
                 pad -> SetPedestalSubtracted(kTRUE);

          for(int iTB=0; iTB<nTBs; iTB++){
            pad -> SetADC(iTB, 0);
          }

          fRawEvent -> SetPad(pad);
          delete pad;
        }
      }
    }
  }

}

ClassImp(STPadResponseTask);
