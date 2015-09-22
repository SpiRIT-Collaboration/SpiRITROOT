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

  fYAnodeWirePlane = fPar->GetAnodeWirePlaneY(); // [mm]
  fZWidthPadPlane  = fPar->GetPadPlaneZ(); // [mm]
  fNumWires        = 363;
  fZSpacingWire    = 4;
  fZOffsetWire     = 2;

  /* 
   *           < CONFIGURATION 1 >              < CONFIGURATION 2 >      \n
   *
   *         -----------------------          -----------------------    \n
   *        |                       |        |                       |   \n
   *        |      1/3     2/3      |        |  1/6     1/2     5/6  |   \n
   *   PAD  |                       |        |                       |   \n
   *        |     across the pad    |        |     across the pad    |   \n
   *        |                       |        |                       |   \n
   *         ---------12 mm---------          -----------------------    \n
   *
   *        |       |       |       |            |       |       |       \n
   *        |       |       |       |            |       |       |       \n
   *  WIRE  |       |  4 mm |       |            |       |       |       \n
   *        |       |       |       |            |       |       |       \n
   *        |       |       |       |            |       |       |       \n
   *
   *
   *                             ---> z-axis (layer)
   *
   *
   *  - CONFIGURATION 1 : Use fZOffsetWire = 0
   *  - CONFIGURATION 2 : Use fZOffsetWire = 2
   *
   *  Default is CONFIGURATION 2.
   */

  fZCenterWire = fZWidthPadPlane/2+fZOffsetWire;
  fZFirstWire  = fZCenterWire - fNumWires/2*fZSpacingWire;
  fZLastWire   = fZCenterWire + fNumWires/2*fZSpacingWire;
  fZCritWire   = fZOffsetWire; // = 0*fZSpacingWire + fZOffsetWire
  fICritWire   = (fZCritWire-fZOffsetWire)/fZSpacingWire; // == 0
  fIFirstWire  = (fZFirstWire-fZOffsetWire)/fZSpacingWire;
  fILastWire   = (fZLastWire-fZOffsetWire)/fZSpacingWire;

  fEIonize  = fPar->GetEIonize()*1.E6; // [MeV] to [eV]
  fVelDrift = fPar->GetDriftVelocity()/100.; // [cm/us] to [mm/ns]
  fCoefT    = fPar->GetCoefDiffusionTrans()*sqrt(10.); // [cm^(-1/2)] to [mm^(-1/2)]
  fCoefL    = fPar->GetCoefDiffusionLong()*sqrt(10.);  // [cm^(-1/2)] to [mm^(-1/2)]
  fGain     = fPar->GetGain();

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
  if(nMCPoints<10){
    fLogger->Warning(MESSAGE_ORIGIN, "Not enough hits for digitization! (<10)");
    fEventID++;
    return;
  }

  /**
   * NOTE! that fMCPoint has unit of [cm] for length scale,
   * [GeV] for energy and [ns] for time.
   */

  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    Double_t eLoss = (fMCPoint->GetEnergyLoss())*1.E9; // [GeV] to [eV]

    Double_t lDrift = fYAnodeWirePlane-(fMCPoint->GetY())*10; // drift length [mm]
    Double_t tDrift = lDrift/fVelDrift; // drift time [ns]
    Double_t sigmaL = fCoefL*sqrt(lDrift); // sigma in longitudinal direction
    Double_t sigmaT = fCoefT*sqrt(lDrift); // sigma in transversal direction

    Int_t nElectrons = (Int_t)floor(fabs(eLoss/fEIonize));
    for(Int_t iElectron=0; iElectron<nElectrons; iElectron++) {
      Int_t gain = gRandom -> Gaus(fGain,20); // TODO : Gain function is neede.
      if(gain<=0) continue;

      Double_t dr    = gRandom->Gaus(0,sigmaT); // displacement in radial direction
      Double_t angle = gRandom->Uniform(2*TMath::Pi()); // random angle

      Double_t dx = dr*TMath::Cos(angle); // displacement in x-direction
      Double_t dz = dr*TMath::Sin(angle); // displacement in y-direction
      Double_t dt = gRandom->Gaus(0,sigmaL)/fVelDrift; // displacement in time

      Int_t iWire = (Int_t)floor((fMCPoint->GetZ()*10+dz+fZSpacingWire/2)/fZSpacingWire);
      if(iWire<fIFirstWire) iWire = fIFirstWire;
      if(iWire>fILastWire)  iWire = fILastWire;
      Int_t zWire = iWire*fZSpacingWire+fZOffsetWire;

      Int_t index = fElectronArray->GetEntriesFast();
      STDriftedElectron *electron
        = new ((*fElectronArray)[index])
          STDriftedElectron(fMCPoint->GetX()*10, dx,
                            fMCPoint->GetZ()*10, dz, 
                            fMCPoint->GetY()*10,
                            fMCPoint->GetTime(), tDrift, dt,
                            iWire, zWire,
                            gain);

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
