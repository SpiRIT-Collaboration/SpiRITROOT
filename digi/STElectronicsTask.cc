/**
 * @brief Simulate pulse signal made in GET electronics. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail See header file for detail.
 */

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

STElectronicsTask::STElectronicsTask()
:FairTask("STElectronicsTask"),
 fEventID(0),
 fADCDynamicRange(120.e-15),
 fADCMax(4095),
 fADCMaxUseable(4095),
 fPedestalMean(400),
 fPedestalSigma(4),
 fPedestalSubtracted(kTRUE),
 fSignalPolarity(1)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STElectronicsTask");
}

STElectronicsTask::~STElectronicsTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STElectronicsTask");
}

void 
STElectronicsTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STElectronicsTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STElectronicsTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STElectronicsTask");

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  fPPEventArray = (TClonesArray*) ioman->GetObject("PPEvent");
  fRawEventArray = new TClonesArray("STRawEvent"); 
  ioman->Register("STRawEvent", "ST", fRawEventArray, fInputPersistance);

  fNTBs = fPar -> GetNumTbs();

  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString pulserFileName = workDir + "/parameters/Pulser.dat";

  fNBinPulser = 0;
  Double_t val;
  ifstream pulserFile(pulserFileName.Data());

  Double_t coulombToEV = 6.241e18; 
  Double_t pulserConstant = (fADCMaxUseable-fPedestalMean)/(fADCDynamicRange*coulombToEV);
  while(pulserFile >> val) fPulser[fNBinPulser++] = pulserConstant*val;

  return kSUCCESS;
}

void 
STElectronicsTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STElectronicsTask");

  if(!fRawEventArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No RawEventArray!");

  fPPEvent  = (STRawEvent*) fPPEventArray -> At(0);

  Int_t nPads = fPPEvent -> GetNumPads();

  fRawEvent = new STRawEvent();
  fRawEvent -> SetEventID(fPPEvent->GetEventID());

  STPad* padI;

  Double_t *adcI;
  Double_t adcO[512];

  for(Int_t iPad=0; iPad<nPads; iPad++) {
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
    if(fPedestalSubtracted) {
      for(Int_t iTB=0; iTB<fNTBs; iTB++){
        if(adcO[iTB]>(fADCMaxUseable-fPedestalMean)) adcO[iTB] = fADCMaxUseable;
      }
    }
    else {
      for(Int_t iTB=0; iTB<fNTBs; iTB++) {
        adcO[iTB] += gRandom -> Gaus(fPedestalMean,fPedestalSigma);
        if(adcO[iTB]>fADCMaxUseable) adcO[iTB] = fADCMaxUseable;
      }
    }
    // Polarity 
    if(fSignalPolarity==0) {
      for(Int_t iTB=0; iTB<fNTBs; iTB++){
        adcO[iTB] = fADCMaxUseable - adcO[iTB];
      }
    }
    // Set ADC
    for(Int_t iTB=0; iTB<fNTBs; iTB++)
      padO -> SetADC(iTB,adcO[iTB]);
    fRawEvent -> SetPad(padO);
    delete padO;
  }

  new ((*fRawEventArray)[0]) STRawEvent(fRawEvent);
  delete fRawEvent;

  fLogger->Info(MESSAGE_ORIGIN, 
                Form("Event #%d : Raw Event created.",
                     fEventID++, nPads));

  return;
}

void STElectronicsTask::SetDynamicRange(Double_t val)      {    fADCDynamicRange = val; }
void STElectronicsTask::SetPedestalMean(Double_t val)      {       fPedestalMean = val; }
void STElectronicsTask::SetPedestalSigma(Double_t val)     {      fPedestalSigma = val; }
void STElectronicsTask::SetPedestalSubtraction(Bool_t val) { fPedestalSubtracted = val; }
void STElectronicsTask::SetSignalPolarity(Bool_t val)      {     fSignalPolarity = val; }


ClassImp(STElectronicsTask)
