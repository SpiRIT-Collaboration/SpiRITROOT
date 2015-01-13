#include "STPadResponse.hh"

#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"

#include <iostream>
#include <fstream>

#include <omp.h>

using namespace std;

ClassImp(STPadResponse);

STPadResponse::STPadResponse()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");

  fNTBs = fPar -> GetNumTbs();

  fTimeMax = 10000.;    //[ns]

  fXPadPlane = (fPar -> GetPadPlaneX()); // [mm]
  fZPadPlane = (fPar -> GetPadPlaneZ()); // [mm]

  fBinSizeX = 8;   //[mm]
  fBinSizeZ = 12;  //[mm]

  fNBinsX = fXPadPlane/fBinSizeX; // 108
  fNBinsZ = fZPadPlane/fBinSizeZ; // 112

  // type 0 (tot=.982)
  fFillRatio[0][0] = 0.491;
  fFillRatio[0][1] = 0.491;
  fFillRatio[0][2] = 0;

  // type 1 (tot=.999)
  fFillRatio[1][0] = 0.215;
  fFillRatio[1][1] = 0.733;
  fFillRatio[1][2] = 0.051;

  // type 2 (tot=.999)
  fFillRatio[2][0] = 0.051;
  fFillRatio[2][1] = 0.733;
  fFillRatio[2][2] = 0.215;

  fPadResponseFunction1
    = new TF1("fPadResponseFunction", this, &STPadResponse::fPadResponseFunction,
              -fXPadPlane/2, fXPadPlane/2, 1, "STPadResponse", "fPadResponseFunction");

  TString  workDir = gSystem -> Getenv("SPIRITDIR");
  TString  integralDataName = workDir + "/parameters/PadResponseIntegral.dat";
  ifstream integralData(integralDataName.Data());

  Double_t x, val;
  Int_t nPoints = 0;

  fPadResponseIntegralData = new TGraph();
  while(integralData >> x >> val) fPadResponseIntegralData -> SetPoint(nPoints++, x, val);

  integralData.close();
}

void 
STPadResponse::FindPad(Double_t xElectron, 
                          Int_t zWire, 
                          Int_t &row, 
                          Int_t &layer, 
                          Int_t &type)
{
  row   = floor(xElectron/fBinSizeX) + fNBinsX/2;
  layer = floor(zWire/fBinSizeZ);

  if(row<0 || row>fNBinsX || layer<0 || layer>fNBinsZ) {
    type=-1;
    return;
  }

  Int_t d = zWire%fBinSizeZ;

       if(d==0) type=0;
  else if(d==4) type=1;
  else if(d==8) type=2;
}


/*
void STPadResponse::FillPad(Int_t gain, Double_t x, Double_t t, Int_t zWire)
{
  Int_t row, layer, type;
  FindPad(x, zWire, row, layer, type);

  if(type==-1) return;

  fPadResponseFunction1 -> SetParameter(0,x);
  Int_t iTB = floor( t * fNTBs / fTimeMax );

  for(Int_t iLayer=0; iLayer<3; iLayer++){ Int_t jLayer = layer+iLayer-1;
  for(Int_t iRow=0;   iRow<5;   iRow++)  { Int_t jRow   = row+iRow-2;

    STPad* pad = fRawEvent -> GetPad(jRow*112+jLayer);
    //STPad* pad = fRawEvent -> GetPad(jRow,jLayer);
    if(!pad) continue;

    Double_t x1 = jRow*fBinSizeX - fXPadPlane/2;
    Double_t x2 = (jRow+1)*fBinSizeX - fXPadPlane/2;
    Double_t content = gain*fFillRatio[type][iLayer]*(fPadResponseFunction1->Integral(x1,x2));
    pad -> SetADC(iTB, content + (pad -> GetADC(iTB)) );

  }
  }

}
*/

void STPadResponse::FillPad(Int_t gain, Double_t x, Double_t t, Int_t zWire)
{
  Int_t row, layer, type;
  FindPad(x, zWire, row, layer, type);

  if(type==-1) return;

  Int_t iTB = floor( t * fNTBs / fTimeMax );

  for(Int_t iLayer=0; iLayer<3; iLayer++){ Int_t jLayer = layer+iLayer-1;
  for(Int_t iRow=0;   iRow<5;   iRow++)  { Int_t jRow   = row+iRow-2;

    //totally depends on how you set pads in STRawEvent
    STPad* pad = fRawEvent -> GetPad(jRow*112+jLayer);
    if(!pad) continue;

    Double_t x1 = jRow*fBinSizeX - fXPadPlane/2;
    Double_t x2 = (jRow+1)*fBinSizeX - fXPadPlane/2;
    Double_t content = gain*fFillRatio[type][iLayer]
                      *(fPadResponseIntegralData -> Eval(x2-x)
                       -fPadResponseIntegralData -> Eval(x1-x));
                      //*(TMath::Erf((x2-x)/5.85941)/2
                      // -TMath::Erf((x1-x)/5.85941)/2);
    pad -> SetADC(iTB, content + (pad -> GetADC(iTB)) );

  }
  }
}

Double_t STPadResponse::fPadResponseFunction(Double_t *x, Double_t *par)
{
  Double_t val = 0.0874277
                *( TMath::ATan(0.868044*TMath::TanH(0.889*((x[0]-par[0])/4-1)))
                  -TMath::ATan(0.868044*TMath::TanH(0.889*((x[0]-par[0])/4+1))) );

  return -val;
}
