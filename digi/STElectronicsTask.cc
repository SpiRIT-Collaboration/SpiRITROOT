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
#include "FairMCEventHeader.h"

// ST header
#include "STElectronicsTask.hh"
#include "STProcessManager.hh"
#include "STPulse.hh"
#include "STFairMCEventHeader.hh"

// C/C++ class headers
#include <iostream>
#include <fstream>

#include "TRandom.h"

using namespace std;

STElectronicsTask::STElectronicsTask()
:FairTask("STElectronicsTask"),
  fIsPersistence(kFALSE),
  fEventID(0),
  fPulseFileName(""),
  fUseSaturationTemplate(kTRUE),
  fSaturatedPulseFileName("saturatedPulse.dat"),
  fADCConstant(0.2),
  fADCDynamicRange(120.e-15),
  fADCMax(4095),
  fADCMaxUseable(4095),
  fPedestalMean(300),
  fPedestalSigma(6),
  fPedestalSubtracted(kTRUE),
  fSignalPolarity(1),
  fKillAfterSaturation(kTRUE),
  fStartTb(0),
  fEndTb(-1),
  fGainMatchingDataScale(112, std::vector<double>(108, 1))
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

  fMCTrackArray = (TClonesArray*) ioman->GetObject("PrimaryTrack");
  fPPEventArray = (TClonesArray*) ioman->GetObject("PPEvent");
  fRawEventArray = new TClonesArray("STRawEvent"); 
  ioman->Register("STRawEvent", "ST", fRawEventArray, fIsPersistence);
  ioman->Register("STMCTrack", "ST", fMCTrackArray, fIsPersistence);

  auto fairMCEventHeader = (FairMCEventHeader*) ioman->GetObject("MCEventHeader.");
  if(auto castedEventHeader = dynamic_cast<STFairMCEventHeader*>(fairMCEventHeader))
    ioman->Register("MCEventHeader.", "ST", castedEventHeader, fIsPersistence);
  else
    ioman->Register("MCEventHeader.", "ST", fairMCEventHeader, fIsPersistence);

 
  fNTBs = fPar -> GetNumTbs();
  if(fEndTb == -1)
    fEndTb = fNTBs;

  fNBinPulser = 0;
  STPulse *stpulse;
  if(fPulseFileName.IsNull())
    stpulse = new STPulse();
  else
    stpulse = new STPulse(fPulseFileName);

  Double_t coulombToEV = 6.241e18; 
  Double_t pulserConstant = fADCConstant*(fADCMaxUseable-fPedestalMean)/(fADCDynamicRange*coulombToEV);
  for(Int_t i=0; i<200; i++)
    fPulser[fNBinPulser++] = pulserConstant * stpulse -> Pulse(i, 1, 0);
  delete stpulse;

  fNBinSaturatedPulse=0;
  STPulse *satpulse = new STPulse(fSaturatedPulseFileName);
  Int_t ADCdynamicrange = fPedestalSubtracted ? fADCMaxUseable-fPedestalMean : fADCMaxUseable;
  for(Int_t i=0; i<256; i++)
    fSaturatedPulse[fNBinSaturatedPulse++] = satpulse -> Pulse(i, ADCdynamicrange, 0);
  delete satpulse;

  fRawEvent = new ((*fRawEventArray)[0]) STRawEvent();


  if (!(fGainMatchingData.IsNull()))
    {
      
      std::ifstream matchList(fGainMatchingData.Data());
      if(matchList.is_open()) 
      {
        fLogger->Info(MESSAGE_ORIGIN, (" Gain matching file " + fGainMatchingData + " set!"));
        Int_t layer = 0;
        Int_t row = 0;
        Double_t relativeGain = 0;
        std::string line;
        while(std::getline(matchList, line))
        {
          double col1, col2, col3;
          std::stringstream ss(line);
          ss >> col1 >> col2;
          layer = int(col1 + 0.5);
          if(ss >> col3)
          {
            row = int(col2 + 0.5);
            fGainMatchingDataScale[layer][row] = col3;
          }
          else 
            for(int iRow = 0; iRow < 108; ++iRow)
              fGainMatchingDataScale[layer][iRow] = col2;
    
        }
      } else fLogger->Info(MESSAGE_ORIGIN, (" Cannot load gain matching file " + fGainMatchingData + ". Gain matching is ignored"));
    }



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

  fRawEvent -> Clear();
  fEventID = fPPEvent->GetEventID();
  fRawEvent -> SetEventID(fEventID);

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

    Double_t satThreshold;
    if(fPedestalSubtracted)
      satThreshold = fADCMaxUseable-fPedestalMean;
    else{
      satThreshold = fADCMaxUseable;
      for(Int_t iTB=0; iTB<fNTBs; iTB++)
	adcO[iTB]+=fPedestalMean;
    }

    // handling the saturation.
    bool is_sat = false;
    Int_t satTB=-1;

    if(fUseSaturationTemplate){
      if(fKillAfterSaturation){

	if(adcO[0]>satThreshold&&adcO[1]>satThreshold)
	  satTB=0;

	Int_t currentADC, previousADC, nextADC;
        Int_t raising_edge_pulse = 0;
	for(Int_t iTB=1; iTB<fNTBs-1; iTB++){
          if(adcO[iTB] < 100) raising_edge_pulse = iTB;
	  currentADC=adcO[iTB];
	  previousADC=adcO[iTB-1];
	  nextADC=adcO[iTB+1];

          // looking for the raisinig edge to embed a saturated pulse
          // Two criteria: If there are no pulse in front of the sat pulse, we subsitute the pulse with the raising edge of the saturated pulse at time bucket when adc channel < 100
          // Second: If there is a pulse infront, it will subsitute at the trough between the two pulses
          if(currentADC < 100) raising_edge_pulse = iTB;
          else if(nextADC < currentADC && currentADC < previousADC) raising_edge_pulse = iTB;

	  if(previousADC<=satThreshold && currentADC>satThreshold && nextADC>satThreshold){
	    satTB=raising_edge_pulse+1;
	    break;		// search first saturation point.
	  }
	}

	if(satTB!=-1){	// found saturation.
          is_sat = true;
	  Int_t tempIndex;	// saturation moment of the template.
          // embed the pulse index according to the pulse height at saturation
          double sat_height = adcO[satTB];
          if(sat_height < 94) tempIndex = 48;
          else if(sat_height < 555) tempIndex = 49;
          else if(sat_height < 1890) tempIndex = 50;
          else tempIndex = 51;
	  while( satTB<fNTBs && tempIndex<fNBinSaturatedPulse )
	    adcO[satTB++] = fSaturatedPulse[tempIndex++];
	  while(satTB<fNTBs)
	    adcO[satTB++] = fSaturatedPulse[tempIndex];
	}
	else{	// there are no adc which exceed the threshold continuously (at least one Tb excess.).
	  for(Int_t iTB=0; iTB<fNTBs; iTB++)
	    if(adcO[iTB]>satThreshold)
	      adcO[iTB]=satThreshold;
	}
      }
    }
    else{
      for(Int_t iTB=0; iTB<fNTBs; iTB++)
	if(adcO[iTB]>satThreshold)
	  adcO[iTB]=satThreshold;
    }


    // Polarity 
    if(fSignalPolarity==0) {
      for(Int_t iTB=0; iTB<fNTBs; iTB++){
	adcO[iTB] = fADCMaxUseable - adcO[iTB];
      }
    }

    // Set ADC
    Int_t row   = padI -> GetRow();
    Int_t layer = padI -> GetLayer();
    STPad *padO = new STPad(row, layer);
    Double_t gainI = fGainMatchingDataScale[layer][row];
    padO -> SetPedestalSubtracted();

    // pad I saturats only if it is killed by STSimulateBeamTask
    if(padI -> IsSaturated())
    { is_sat = true; satTB = 0; }

    for(Int_t iTB=fStartTb; iTB<fEndTb; iTB++){
      adcO[iTB] += gRandom -> Gaus(0,fPedestalSigma);

      padO -> SetADC(iTB,adcO[iTB]*gainI);
    }
    if(is_sat)
    {
      padO -> SetIsSaturated(is_sat);
      padO -> SetSaturatedTb(satTB);
      padO -> SetSaturatedTbMC(satTB);
    }
    fRawEvent -> SetPad(padO);
    delete padO;
  }

  fLogger->Info(MESSAGE_ORIGIN, 
      Form("Event #%d : Raw Event created.",
	fEventID));

  return;
}

void STElectronicsTask::SetPersistence(Bool_t value)       {    fIsPersistence = value; }
void STElectronicsTask::SetADCConstant(Double_t val)       {       fADCConstant  = val; }
void STElectronicsTask::SetPulseData(TString val)          {     fPulseFileName  = val; }
void STElectronicsTask::SetDynamicRange(Double_t val)      {    fADCDynamicRange = val; }
void STElectronicsTask::SetPedestalMean(Double_t val)      {       fPedestalMean = val; }
void STElectronicsTask::SetPedestalSigma(Double_t val)     {      fPedestalSigma = val; }
void STElectronicsTask::SetPedestalSubtraction(Bool_t val) { fPedestalSubtracted = val; }
void STElectronicsTask::SetSignalPolarity(Bool_t val)      {     fSignalPolarity = val; }

void STElectronicsTask::SetUseSaturationTemplate(Bool_t val) {  fUseSaturationTemplate = val; }
void STElectronicsTask::SetSaturatedPulseData(TString val)   { fSaturatedPulseFileName = val; }
void STElectronicsTask::SetIsKillAfterSaturation(Bool_t val) {    fKillAfterSaturation = val; }
void STElectronicsTask::SetTbRange(Int_t s, Int_t e)         {    fStartTb = s; fEndTb = e;   }

ClassImp(STElectronicsTask)
