#include "iostream"
#include "TRandom.h"
#include "STDriftElectron.hh"

#include "FairRunAna.h"
#include "FairRuntimeDb.h"

using std::cout;
using std::endl;

ClassImp(STDriftElectron);

STDriftElectron::STDriftElectron()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
  fGas = fPar -> GetGas();

  fWirePlaneY = (fPar -> GetAnodeWirePlaneY())/10; // [mm] to [cm]
  fVelDrift   = (fGas -> GetDriftVelocity())/1000; // [cm/us] to [cm/ns]
  fCoefDL     = fGas -> GetCoefDiffusionLong();
  fCoefDT     = fGas -> GetCoefDiffusionTrans();
}

void STDriftElectron::SetMCHit(TLorentzVector positionTimeMC)
{
  fPositionTimeMC = positionTimeMC;
  // Drift lenght [cm] and time [ns]
  y      = fPositionTimeMC.Y();
  lDrift = fWirePlaneY - y;
  tDrift = lDrift / fVelDrift;

  // Sigma calculation
  sigmaDL = fCoefDL * sqrt(lDrift);
  sigmaDT = fCoefDT * sqrt(lDrift);
}

TLorentzVector STDriftElectron::Drift()
{
  // Drift time diffusion [ns]
  dtDrift = (gRandom -> Gaus(0,sigmaDT)) / fVelDrift;

  // Position diffusion (longitudinal) [cm]
  dxz   = gRandom -> Gaus(0,sigmaDL);
  angle = gRandom -> Uniform(2*TMath::Pi());
  dx    = dxz * TMath::Cos(angle); 
  dz    = dxz * TMath::Sin(angle); 

  TLorentzVector 
    positionTimeDrift(fPositionTimeMC.X() + dx,
                      0, // y information is meaningless now
                      fPositionTimeMC.Z() + dz,
                      fPositionTimeMC.T() + tDrift + dtDrift);

  return positionTimeDrift;
}
