#include "STPulse.hh"
#include "TSystem.h"
#include <fstream>
#include <iostream>

using namespace std;

ClassImp(STPulse)

STPulse::STPulse()
{
  TString sysFile = gSystem -> Getenv("VMCWORKDIR");
  TString fileName = sysFile + "/parameters/PulserFitted.dat";
  Initialize(fileName);
}

STPulse::STPulse(TString fileName, Double_t step)
{
  fStep = step;
  Initialize(fileName);
}

void STPulse::Initialize(TString fileName)
{
  std::ifstream file(fileName);

  Double_t value;
  Double_t max = 0;

  while (file >> value) {
    if (max < value) {
      max = value; 
      fTbAtMax = fNumDataPoint * fStep;
    }
    fNumDataPoint++;
  }

  if (fNumDataPoint == 0 || max == 0) {
    cout << "*** Error occured while initializing the pulse!" << endl;
    cout << "*** Check the file: " << fileName << endl;
    return;
  }

  Double_t c = 1;
  if (max != 1)
    c = 1./max;

  fPulseData = new Double_t[fNumDataPoint];

  file.clear();
  file.seekg(0, ios::beg);

  for(Int_t iData = 0; iData < fNumDataPoint; iData++){
    file >> value;
    fPulseData[iData] = c * value;
  }

  file.close();
}

Double_t 
STPulse::Pulse(Double_t x, Double_t amp, Double_t tb0)
{
  Double_t tb = x - tb0;
  if (tb < 0) 
    return 0;

  Int_t tbInStep = tb / fStep;

  if (tbInStep > fNumDataPoint - 2) 
    return 0;

  Double_t r = (tb / fStep) - tbInStep;
  Double_t val = r * fPulseData[tbInStep + 1] + (1 - r) * fPulseData[tbInStep];

  return amp * val;
}

Double_t 
STPulse::PulseF1(Double_t *x, Double_t *par)
{
  Double_t tb = x[0] - par[1];
  if (tb < 0) 
    return 0;

  Int_t tbInStep = tb / fStep;
  if (tbInStep > fNumDataPoint - 2)
    return 0;

  Double_t r = (tb / fStep) - tbInStep;
  Double_t val = r * fPulseData[tbInStep + 1] + (1 - r) * fPulseData[tbInStep];

  return par[0] * val;
}

TF1*
STPulse::GetPulseFunction(TString name)
{
  if (name.IsNull()) 
    name = Form("STPulse_%d", fNumF1++);
  TF1* f1 = new TF1(name, this, &STPulse::PulseF1, 0, 512, 2, "STPulse", "PulseF1");
  return f1;
}

TF1*
STPulse::GetPulseFunction(STHit* hit, TString name)
{
  TF1* f1 = GetPulseFunction(name);
  f1 -> SetParameters(hit -> GetCharge(), hit -> GetTb());
  return f1;
}
