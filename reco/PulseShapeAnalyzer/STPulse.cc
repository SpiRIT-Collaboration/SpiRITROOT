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

STPulse::STPulse(TString fileName)
{
  Initialize(fileName);
}

void STPulse::Initialize(TString fileName)
{
  std::ifstream file(fileName);

  Double_t value;
  Double_t max = 0;
  fNumDataPoints = 0;

  file >> fStep;
  while (file >> value) {
    if (max < value) {
      max = value; 
      fTbAtMax = fNumDataPoints * fStep;
    }
    fNumDataPoints++;
  }

  if (fNumDataPoints == 0 || max == 0) {
    cout << "*** Error occured while initializing the pulse!" << endl;
    cout << "*** Check the file: " << fileName << endl;
    return;
  }

  Double_t c = 1;
  if (max != 1)
    c = 1./max;

  fPulseData = new Double_t[fNumDataPoints];

  file.clear();
  file.seekg(0, ios::beg);

  file >> fStep;
  for(Int_t iData = 0; iData < fNumDataPoints; iData++){
    file >> value;
    fPulseData[iData] = c * value;
  }

  file.close();

  fNumAscending = 0;
  fThresholdTbStep = 5000.;

  Double_t valuePre = 0, valueCur = 0;
  Bool_t ascending = kFALSE;
  for(Int_t iData = 0; iData < fNumDataPoints;)
  {
    valuePre = valueCur;
    valueCur = fPulseData[iData];
    Double_t tb = iData * fStep;

    if (ascending == kFALSE && valueCur > fThresholdRatio) {
      ascending = kTRUE;
      fTbAtThreshold = tb;
      iData += Int_t(1/fStep);
      continue;
    }
    else if (ascending == kTRUE && valueCur > 0.9)
      break;

    if (ascending)
    {
      Double_t valDiff = valueCur - valuePre;
      if (valDiff < fThresholdTbStep)
        fThresholdTbStep = valDiff;
      fNumAscending++;
      iData += Int_t(1/fStep);
    }
    else
      iData += 1;
  }

  fNumF1 = 0;

  if (fNumAscending < 2)
    fNumAscending = 2;
}

Double_t 
STPulse::Pulse(Double_t x, Double_t amp, Double_t tb0)
{
  Double_t tb = x - tb0;
  if (tb < 0) 
    return 0;

  Int_t tbInStep = tb / fStep;

  if (tbInStep > fNumDataPoints - 2) 
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
  if (tbInStep > fNumDataPoints - 2)
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

Double_t  STPulse::GetTbAtThreshold()   { return fTbAtThreshold;   }
Double_t  STPulse::GetTbAtMax()         { return fTbAtMax;         }
   Int_t  STPulse::GetNumAscending()    { return fNumAscending;    }
Double_t  STPulse::GetThresholdTbStep() { return fThresholdTbStep; }
   Int_t  STPulse::GetNumDataPoints()   { return fNumDataPoints;   }
Double_t *STPulse::GetPulseData()       { return fPulseData;       }

void 
STPulse::Print()
{
  cout << "[STPulse INFO]" << endl;
  cout << " == Number of data points : " << fNumDataPoints << endl;
  cout << " == Step size between data points : " << fStep << endl;
  cout << " == Threshold for one timebucket step : " << fThresholdTbStep << endl; 
  cout << " == Number of timebucket while rising : " << fNumAscending << endl;
  cout << " == Timebucket at threshold (" << setw(3) << fThresholdRatio 
       << " of peak) : " << fTbAtThreshold << endl; 
  cout << " == Timebucket at peak : " << fTbAtMax << endl; 
  cout << " == Timebucket difference from threshold to peak : " 
       << fTbAtMax - fTbAtThreshold << endl; 
}
