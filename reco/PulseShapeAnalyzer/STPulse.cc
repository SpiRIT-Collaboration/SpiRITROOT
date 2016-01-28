#include "STPulse.hh"
#include "TSystem.h"
#include <fstream>
#include <iostream>

using namespace std;

ClassImp(STPulse)

STPulse::STPulse()
{
  fPulseData = new Double_t[2000];

  TString sysFile = gSystem -> Getenv("VMCWORKDIR");
  TString fileName = sysFile + "/parameters/PulserFitted.dat";
  std::ifstream file(fileName);

  Int_t countData = 0;
  while (file >> fPulseData[countData++]) {}

  file.close();
}

Double_t 
STPulse::Pulse(Double_t x, Double_t amp, Double_t tb)
{
  if (x < tb) 
    return 0;

  Double_t hit_time = x - tb;
  Int_t hit_time_0p1 = hit_time * 10;

  if (hit_time_0p1 > 1998) 
    return 0;

  Double_t r = 10 * hit_time - hit_time_0p1;
  Double_t val = r * fPulseData[hit_time_0p1 + 1] + (1 - r) * fPulseData[hit_time_0p1];

  return amp * val;
}

Double_t 
STPulse::PulseF1(Double_t *x, Double_t *par)
{
  //x[0] += 0.5;

  if (x[0] < par[1]) 
    return 0;

  Double_t hit_time = x[0] - par[1];
  Int_t hit_time_0p1 = hit_time * 10;
  if (hit_time_0p1 > 1998) 
    return 0;

  Double_t r = 10 * hit_time - hit_time_0p1;
  Double_t val = r * fPulseData[hit_time_0p1 + 1] + (1 - r) * fPulseData[hit_time_0p1];

  return par[0] * val;
}

TF1*
STPulse::GetPulseFunction(TString name)
{
  TF1* f1 = new TF1(name, this, &STPulse::PulseF1, 0, 512, 2, "STPulse", "PulseF1");
  return f1;
}

TF1*
STPulse::GetPulseFunction(STHit* hit, TString name)
{
  if (name.IsNull()) 
    name = Form("STPulse_%d", fIndex++);
  TF1* f1 = GetPulseFunction(name);
  f1 -> SetParameters(hit -> GetCharge(), hit -> GetTb());
  return f1;
}
