#include "TRandom.h"

#include "STWireResponse.hh"
#include "FairRootManager.h"
#include "iostream"

using std::cout;
using std::endl;

ClassImp(STWireResponse);

STWireResponse::STWireResponse(STGas* gas)
: fGas(gas)
{
  // wire plane 
  nWire = 363;
  zCenterWire  = 67.2; // [cm]
  zSpacingWire = 0.4;  // [cm]

  zFirstWire = zCenterWire - (nWire-1)/2 * zSpacingWire;
  zLastWire  = zCenterWire + (nWire-1)/2 * zSpacingWire;

  // pad plane 
  Double_t fPadPlaneX = 96.61;
  Double_t fPadPlaneZ = 144.64;

  fPadPlane 
    = new TH2D("ElDistXZAval","", 112,0,fPadPlaneZ,
                                  108,-fPadPlaneX/2,fPadPlaneX/2);

  fWPField 
    = new TF2("WPField",
              "[0]*TMath::Gaus(x,[1],[2],1)*TMath::Gaus(y,[3],[4],1)",
              0,fPadPlaneX,-fPadPlaneZ/2,fPadPlaneZ/2);
  // [0] : amplitude
  // [1] : meanX
  // [2] : sigmaX
  // [3] : meanZ
  // [4] : sigmaZ
  fWPField -> SetParameters(1,0,1,0,1);
  
  fGain = fGas -> GetGain();
}

Double_t STWireResponse::FindZWire(Double_t z)
{
  iWire = floor( (z - zCenterWire + zSpacingWire/2) / zSpacingWire );
  zWire = iWire * zSpacingWire + zCenterWire;

  return zWire;
}

Int_t STWireResponse::FillPad(Double_t x)
{
  gain = gRandom -> Gaus(fGain,fGain/100);
  if(gain<0) gain = 0;

  fWPField  -> SetParameters(gain,x,0.1,zWire,0.1);
  fPadPlane -> FillRandom("WPField",gain);

  return gain;
}

void STWireResponse::WriteHistogram()
{
  TFile* file = FairRootManager::Instance() -> GetOutFile();

  file -> mkdir("PadPlane");
  file -> cd("PadPlane");

  fPadPlane -> Write();
}
