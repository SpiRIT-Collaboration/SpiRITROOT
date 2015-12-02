#include "STPulse.hh"
#include "TSystem.h"
#include <fstream>

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
STPulse::Pulse(Double_t *x, Double_t *par)
{
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
