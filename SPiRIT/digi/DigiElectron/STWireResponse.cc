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
  maxTime = 10000.;    //[ns]

  // wire plane 
  Int_t nWire = 363;
  zCenterWire  = 67.2; // [cm]
  zSpacingWire = 0.4;  // [cm]

  zFirstWire = zCenterWire - (nWire-1)/2 * zSpacingWire;
  zLastWire  = zCenterWire + (nWire-1)/2 * zSpacingWire;

  // pad plane 
  xPadPlane = 96.61;
  zPadPlane = 144.64;

  binSizeZ = zPadPlane/112;
  binSizeX = xPadPlane/108;

  fPadPlane 
    = new TH2D("ElDistXZAval","", 112,0,zPadPlane,
                                  108,-xPadPlane/2,xPadPlane/2);

  fWPField = new TF2("WPField", this, &STWireResponse::WPField,
                     Double_t(0), zPadPlane, -xPadPlane/2, xPadPlane/2, 5,
                     "STWireReponse", "WPField");
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
  Int_t iWire = floor( (z - zCenterWire + zSpacingWire/2) / zSpacingWire );
  zWire = iWire * zSpacingWire + zCenterWire;

  if(zWire>zLastWire)  zWire = zLastWire;
  if(zWire<zFirstWire) zWire = zFirstWire;

  return zWire;
}

Int_t STWireResponse::FillPad(Double_t x, Double_t t)
{
  gain = gRandom -> Gaus(fGain,fGain/100);
  if(gain<0) return 0;

  fWPField -> SetParameters(gain,zWire,0.2,x,0.2);

  Int_t bin = fPadPlane -> Fill(zWire,x,0);
  Int_t binZ, binX, dummy;
  fPadPlane -> GetBinXYZ(bin, binZ, binX, dummy);

  if(binZ<0 || binZ>112 || binX<0 || binX>108) return 0;

  Double_t z1;
  Double_t z2;
  Double_t x1;
  Double_t x2;
  Double_t content;

  z1 = binZ*binSizeZ;
  z2 = (binZ+1)*binSizeZ;
  x1 = binX*binSizeX - xPadPlane/2;
  x2 = (binX+1)*binSizeX - xPadPlane/2;

  content  = fWPField -> Integral(z1,z2,x1,x2);

  Int_t iTB = floor( t * nTBs / maxTime );

  pad = fEvent -> GetPad(binX,binZ);
  pad -> SetADC(iTB, content + (pad -> GetADC(iTB)) );

  fPadPlane -> SetBinContent(binZ,binX, content + (fPadPlane -> GetBinContent(binZ,binX)) );


  /*
  for(Int_t dZ=-1; dZ<=1; dZ++){
    for(Int_t dX=-1; dX<=1; dX++){

      Int_t ibinZ = binZ+dZ;
      Int_t ibinX = binX+dX;

      z1 = ibinZ*binSizeZ;
      z2 = (ibinZ+1)*binSizeZ;
      x1 = ibinX*binSizeX - xPadPlane/2;
      x2 = (ibinX+1)*binSizeX - xPadPlane/2;

      content  = fWPField -> Integral(z1,z2,x1,x2);
      content += fPadPlane -> GetBinContent(ibinZ,ibinX);
    }
  }
  */

  return gain;
}

void STWireResponse::WriteHistogram()
{
  TFile* file = FairRootManager::Instance() -> GetOutFile();

  file -> mkdir("PadPlane");
  file -> cd("PadPlane");

  fPadPlane -> Write();
}

Double_t STWireResponse::WPField(Double_t *x, Double_t *par)
{
  Float_t xx = x[0];
  Float_t yy = x[1];

  Double_t val = par[0]*TMath::Gaus(xx,par[1],par[2],kTRUE)
                       *TMath::Gaus(yy,par[3],par[4],kTRUE);

  return val;
}
