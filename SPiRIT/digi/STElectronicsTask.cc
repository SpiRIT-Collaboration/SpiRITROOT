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
#include "STProcessManager.hh"

// C/C++ class headers
#include <iostream>
#include <fstream>

#include "TRandom.h"

using namespace std;

// ---- Default constructor -------------------------------------------
STElectronicsTask::STElectronicsTask()
  :FairTask("STElectronicsTask"),
   fADCDynamicRange(120.e-15),
   fADCMax(4095),
   fADCMaxUseable(3600),
   fADCPedestal(400),
   fSignalPolarity(0)
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

  fPPEventArray = (TClonesArray*) ioman->GetObject("PPEvent");
  fRawEventArray = new TClonesArray("STRawEvent"); 
  ioman->Register("STRawEvent", "ST", fRawEventArray, kTRUE);

  fNTBs = fPar -> GetNumTbs(); // number of time buckets

  TString workDir = gSystem -> Getenv("SPIRITDIR");
  TString pulserFileName = workDir + "/parameters/Pulser.dat";

  fNBinPulser = 0;
  Double_t val;
  ifstream pulserFile(pulserFileName.Data());

  Double_t coulombToEV = 6.241e18; 
  Double_t pulserConstant = (fADCMaxUseable-fADCPedestal)/(fADCDynamicRange*coulombToEV);
  while(pulserFile >> val) fPulser[fNBinPulser++] = pulserConstant*val;

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

  if(!fRawEventArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No RawEventArray!");

  fLogger->Info(MESSAGE_ORIGIN, "Pad plane event found.");

  fPPEvent  = (STRawEvent*) fPPEventArray -> At(0);
  fRawEvent = new STRawEvent();
  fRawEvent -> SetEventID(fPPEvent->GetEventID());

  STPad* padI;

  Double_t *adcI;
  Double_t adcO[512];

  Int_t nPads = fPPEvent -> GetNumPads();
  STProcessManager fProcess("Electronics", nPads);
  for(Int_t iPad=0; iPad<nPads; iPad++) {
    fProcess.PrintOut(iPad);
    padI = fPPEvent -> GetPad(iPad);
    adcI = padI -> GetADC();
    for(Int_t iTB=0; iTB<fNTBs; iTB++) adcO[iTB]=0;
    for(Int_t iTB=0; iTB<fNTBs; iTB++) {
      Double_t val = adcI[iTB];
      Int_t jTB=iTB;
      Int_t kTB=0;
      while(jTB<fNTBs && kTB<fNBinPulser) 
        adcO[jTB++] += val*fPulser[kTB++];
    }
    Int_t row   = padI -> GetRow();
    Int_t layer = padI -> GetLayer();
    STPad *padO = new STPad(row, layer);
    padO -> SetPedestalSubtracted();
    // AGET chip protection from ZAP board
    for(Int_t iTB=0; iTB<fNTBs; iTB++) {
      adcO[iTB] += gRandom -> Gaus(fADCPedestal,4);
      if(adcO[iTB]>fADCMaxUseable) 
        adcO[iTB] = fADCMaxUseable;
    }
    // polarity 
    if(fSignalPolarity==0)
      for(Int_t iTB=0; iTB<fNTBs; iTB++)
        adcO[iTB] = fADCMaxUseable - adcO[iTB];
    // set ADC
    for(Int_t iTB=0; iTB<fNTBs; iTB++)
      padO -> SetADC(iTB,adcO[iTB]);
    fRawEvent -> SetPad(padO);
    delete padO;
  }
  fProcess.End();

  new ((*fRawEventArray)[0]) STRawEvent(fRawEvent);
  delete fRawEvent;

  fLogger->Info(MESSAGE_ORIGIN, "Raw event created.");

  return;
}

// ---- Finish --------------------------------------------------------
void STElectronicsTask::Finish()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Finish of STElectronicsTask");
}

ClassImp(STElectronicsTask)
