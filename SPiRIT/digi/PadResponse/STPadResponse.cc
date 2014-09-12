#include "STPadResponse.hh"
#include "FairRootManager.h"

ClassImp(STPadResponse);

STPadResponse::STPadResponse(STDigiPar* par)
{
  fPar = par;
  fPadPlaneX = fPar -> GetPadPlaneX();
  fPadPlaneZ = fPar -> GetPadPlaneZ();

  fPadSizeX = fPar -> GetPadSizeX();
  fPadSizeZ = fPar -> GetPadSizeZ();

  Int_t numBinX = fPadPlaneX/fPadSizeX;
  Int_t numBinZ = fPadPlaneZ/fPadSizeZ;

  fPadPlane = new TH2D("padplane","", numBinX, -fPadPlaneX/2, fPadPlaneX/2
                                    , numBinZ, 0, fPadPlaneZ);
}

STPadResponse::~STPadResponse()
{
  if(fPadPlane) delete fPadPlane;
}

void STPadResponse::SetPad(STPad* pad, Double_t x, Double_t z, Int_t charge)
{
  Int_t bin;
  Int_t binX;
  Int_t binZ;
  Int_t dummy;

  fPadPlane -> Fill(x,z,charge);
  fPadPlane -> GetBinXYZ(bin, binX, binZ, dummy);

  //pad -> SetRow(binX);
  //pad -> SetLayer(binZ);
  //pad -> SetADC(charge);
}

void STPadResponse::WriteHistogram()
{
  TFile* file = FairRootManager::Instance() -> GetOutFile();

  file -> mkdir("STDriftTask");
  file -> cd("STDriftTask");

  fPadPlane -> Write();
  delete fPadPlane;
}
