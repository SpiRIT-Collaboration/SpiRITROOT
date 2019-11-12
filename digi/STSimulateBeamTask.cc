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
#include "STSimulateBeamTask.hh"
#include "STProcessManager.hh"

// C/C++ class headers
#include <iostream>
#include <fstream>

#include "TRandom.h"

using namespace std;

STSimulateBeamTask::STSimulateBeamTask()
:FairTask("STSimulateBeamTask"),
  fIsPersistence(kFALSE)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STSimulateBeamTask");
}

STSimulateBeamTask::~STSimulateBeamTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STSimulateBeamTask");
}

void 
STSimulateBeamTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STSimulateBeamTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STSimulateBeamTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STSimulateBeamTask");

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  fPPEventArray = (TClonesArray*) ioman->GetObject("PPEvent");
  fPadSizeRow   = fPar -> GetPadSizeX();
  fPadSizeLayer = fPar -> GetPadSizeZ();
  fNRows   = fPar -> GetPadRows(); // 108
  fNLayers = fPar -> GetPadLayers(); // 112

  fNTBs = fPar -> GetNumTbs();

  if((!fDeadOnBeamProbFileName.IsNull()) && (!fDeadOnBeamProbHistName.IsNull()))
  {
    TFile file(fDeadOnBeamProbFileName);
    if(file.IsOpen())
    {
      fDeadOnBeamProb = (TH2F*) file.Get(fDeadOnBeamProbHistName);
      if(fDeadOnBeamProb)
      {
        fDeadOnBeamProb = (TH2F*) fDeadOnBeamProb->Clone("hist");
        fDeadOnBeamProb -> SetDirectory(0);
        fLogger -> Info(MESSAGE_ORIGIN, ("Dead pad probability histogram " + fDeadOnBeamProbHistName + " is loaded from file " + fDeadOnBeamProbFileName));
      }else fLogger -> Info(MESSAGE_ORIGIN, ("Histogram " + fDeadOnBeamProbHistName + " cannot be loaded from file " + fDeadOnBeamProbFileName));
    }else fLogger -> Info(MESSAGE_ORIGIN, ("File " + fDeadOnBeamProbFileName + " cannot be opened"));
  }else fLogger -> Info(MESSAGE_ORIGIN, "No pads will be killed on beam");

  if(fUseHeavyFragment)
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Will simulate heavy fragment along beam line ");
    double VelDrift = fPar -> GetDriftVelocity()/100.; // [cm/us] to [mm/ns]
    double tDrift = fabs(fBeamHeight)/VelDrift + fPar -> GetYDriftOffset();
    fTBAtBeam = tDrift/fPar -> GetTBTime();
  } else fLogger -> Info(MESSAGE_ORIGIN, "Heavy fragment simulation is disabled");

  return kSUCCESS;
}

void 
STSimulateBeamTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STSimulateBeamTask");

  fPPEvent  = (STRawEvent*) fPPEventArray -> At(0);
  Int_t nPads = fPPEvent -> GetNumPads();
  STPad* padI;
  Double_t *adcI;

  for(Int_t iPad=0; iPad<nPads; iPad++) {
    padI = fPPEvent -> GetPad(iPad);
    adcI = padI -> GetADC();

    Int_t layer = padI -> GetLayer();
    Int_t row   = padI -> GetRow();

    if(fUseHeavyFragment)
    {
      double PadZ = (layer + 0.5)*fPadSizeLayer; // 0.5 for mean pad z
      double PadX = (row + 0.5 - fNRows/2)*fPadSizeRow;


      double BeamXAtPadZ = fBeamTrajectory.Eval(PadZ);
      double PulseAmp = fFragmentStrength * TMath::Gaus(PadX, BeamXAtPadZ, fFragmentWidth);
      adcI[fTBAtBeam] = PulseAmp;
    }

    if(fDeadOnBeamProb)
    {
      double prob = fDeadOnBeamProb -> GetBinContent(layer + 1, row + 1); // has to add 1 because bin starts from 1
      if(prob > gRandom->Uniform(0,1)) 
      {
        padI -> SetIsSaturated(true);
        padI -> SetSaturatedTb(0);
        for(Int_t iTB=0; iTB<fNTBs; iTB++)
          adcI[iTB] = 0; // the entire pad is reset if it is dead
      }
    }


  }

  return;
}

void STSimulateBeamTask::SetPersistence(Bool_t value)       {    fIsPersistence = value; }
void STSimulateBeamTask::SetDeadPadOnBeam(TString filename, TString treename)
{ fDeadOnBeamProbFileName = filename; fDeadOnBeamProbHistName = treename; }
void STSimulateBeamTask::SetHeavyFragments(TString filename, double beamheight,
                                           double fragmentStrength, double fragmentWidth)
{
  fUseHeavyFragment = true;
  fBeamHeight = beamheight;
  fFragmentStrength = fragmentStrength;
  fFragmentWidth = fragmentWidth;
  
  // need to invert x-z axis of the graph.
  // temp variable is used to fill fBeamTrajectory;
  fBeamTrajectory.Set(0);
  TGraph temp(filename, "%lg %*lg %lg");
  auto x = temp.GetX();
  auto y = temp.GetY();
  for(int i = 0; i < temp.GetN(); ++i) fBeamTrajectory.SetPoint(fBeamTrajectory.GetN(), y[i], x[i]);
}


ClassImp(STSimulateBeamTask)
