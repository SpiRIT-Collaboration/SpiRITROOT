#include "STPadPlaneConf.hh"

ClassImp(STPadPlaneConf);

STPadPlaneConf* STPadPlaneConf::fInstance=0;
STPadPlaneConf* STPadPlaneConf::Instance()
{
  if(!fInstance) fInstance = new STPadPlaneConf();
  return fInstance;
}

STPadPlaneConf::STPadPlaneConf()
{
  fXPadPlane = 864;
  fZPadPlane = 1344;

  fPadWidthRow = 8;
  fPadWidthLayer = 12;

  fNRows = 108;
  fNLayers = 112;
}

STPadPlaneConf::~STPadPlaneConf()
{
}

Int_t STPadPlaneConf::GetPadWidthRow()   { return fPadWidthRow; }
Int_t STPadPlaneConf::GetPadWidthLayer() { return fPadWidthLayer; }
Int_t STPadPlaneConf::GetNumRows()       { return fNRows; }
Int_t STPadPlaneConf::GetNumLayers()     { return fNLayers; }

Int_t STPadPlaneConf::GetRow(Double_t x)
{ 
  return row = (x+fXPadPlane/2)/(Double_t)fPadWidthRow; 
}

Int_t STPadPlaneConf::GetLayer(Double_t z)
{ 
  return layer = z/(Double_t)fPadWidthLayer; 
}

void STPadPlaneConf::GetRowLayer(Double_t x, Double_t z, Int_t &row, Int_t &layer)
{
  row = (x+fXPadPlane/2)/(Double_t)fPadWidthRow; 
  layer = z/(Double_t)fPadWidthLayer; 
}

void STPadPlaneConf::GetPadCenter(Int_t row, Int_t layer, Double_t& x, Double_t& z)
{
}
