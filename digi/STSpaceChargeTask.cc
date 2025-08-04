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
  if(fUseLocalRate)
    fAuxHeader = (STAuxHeader *) ioman -> GetObject("MCAuxHeader");

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
  fEventID = fFairMCEventHeader -> GetEventID();

  if(fUseLocalRate) 
  {
    auto eventNum = fEventID;
    if(fAuxHeader)
      eventNum = fAuxHeader -> GetTpcEventNum();
    if(!fFirstEventDone || eventNum % fEventFrequency == 0)
    {
      double rate = 0;
      if(eventNum > fRateHist -> GetBinLowEdge(fRateHist -> GetNbinsX()))
        rate = fRateHist -> GetBinContent(fRateHist -> GetNbinsX());
      else 
        rate = fRateHist -> GetBinContent(fRateHist->FindBin(eventNum));
        auto density = fDensityScaleSlope * rate * rate + fDensityScaleInter * rate;
        if(density >= 0) 
        {
          SetSheetChargeDensity(density);
          UpdateEDrift();
        }
      fFirstEventDone = true;
    }
  }

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

  fLogger->Info(MESSAGE_ORIGIN, 
            Form("Event #%d : MC points (%d) found. They are dispaced due to space charge",
                 fEventID, nMCPoints));

  return;
}

void STSpaceChargeTask::SetLocalRate(Double_t scale_s, Double_t scale_i, TString filename, TString histname, Int_t frequency)
{
  fDensityScaleSlope = scale_s;
  fDensityScaleInter = scale_i;
  fEventFrequency = frequency;
  TFile *ratefile = new TFile(filename.Data());
  fRateHist = (TH1D *)ratefile -> Get(histname.Data());
  fUseLocalRate = true;
}

void STSpaceChargeTask::SetLocalRate(Double_t scale_s, Double_t scale_i, Double_t run_rate)
{
  fDensityScaleSlope = scale_s;
  fDensityScaleInter = scale_i;
  auto density = fDensityScaleSlope * run_rate * run_rate + fDensityScaleInter * run_rate;
  this->SetSheetChargeDensity(density);
}

void STSpaceChargeTask::UpdateEDrift()
{
   this -> CalculateEDrift(fPar->GetDriftVelocity());
}

void STSpaceChargeTask::SetPersistence(Bool_t value) { fIsPersistence = value; }
void STSpaceChargeTask::SetVerbose(Bool_t value) { fVerbose = value; }

ClassImp(STSpaceChargeTask);
