/**
 * @brief Calculate position of electron at the anode wire plane passed from 
 * STDriftTask.
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail See header file for detail.
 */

#include "STDriftElectron.hh"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include <iostream>
#include "TRandom.h"

using namespace std;

STDriftElectron::STDriftElectron()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
  fGas = fPar -> GetGas();

  fWirePlaneY = (fPar -> GetAnodeWirePlaneY());
  fVelDrift  = (fGas -> GetDriftVelocity())/100; // [cm/us] to [mm/ns]
  fCoefDL = fGas -> GetCoefDiffusionLong()/sqrt(10);  // [cm^(-1/2)] to [mm^(-1/2)]
  fCoefDT = fGas -> GetCoefDiffusionTrans()/sqrt(10); // [cm^(-1/2)] to [mm^(-1/2)]
}

STDriftElectron::~STDriftElectron()
{
}

void 
STDriftElectron::SetMCHit(TLorentzVector v4MC)
{
  fV4MC = v4MC;

  Double_t driftLength = fWirePlaneY-fV4MC.Y(); // [mm]

  fDriftTime = driftLength / fVelDrift;
  fSigmaDL   = fCoefDL * sqrt(driftLength);
  fSigmaDT   = fCoefDT * sqrt(driftLength);
}

TLorentzVector 
STDriftElectron::Drift()
{
  // units in [mm] and [ns].
  Double_t dt    = (gRandom->Gaus(0,fSigmaDT)) / fVelDrift;
  Double_t dr    = gRandom->Gaus(0,fSigmaDL);
  Double_t angle = gRandom->Uniform(2*TMath::Pi());
  Double_t dx    = dr * TMath::Cos(angle); 
  Double_t dz    = dr * TMath::Sin(angle); 

  TLorentzVector 
    positionTimeDrift(fV4MC.X() + dx,
                      0,
                      fV4MC.Z() + dz,
                      fV4MC.T() + fDriftTime + dt);

  return positionTimeDrift;
}

ClassImp(STDriftElectron);
