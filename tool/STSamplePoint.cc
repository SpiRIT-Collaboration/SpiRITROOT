#include "STSamplePoint.hh"

#include <iostream>
using namespace std;

ClassImp(STSamplePoint)

STSamplePoint::STSamplePoint()
{
  Init(-999, 0, 1);
}

STSamplePoint::STSamplePoint(STSamplePoint &sample)
{
  Init(sample.fValue, sample.fRMS, sample.fWeightSum);
}

STSamplePoint::STSamplePoint(Double_t v, Double_t rms, Double_t w)
{
  Init(v, rms, w);
}

STSamplePoint::~STSamplePoint()
{
}

void 
STSamplePoint::Print()
{
  cout << " STSamplePoint " << fValue << " | " << fRMS << " | " << fWeightSum << endl;
}

void 
STSamplePoint::Init(Double_t v, Double_t rms, Double_t w)
{
  fValue = v;
  fRMS = rms;
  fWeightSum = w;
}

void 
STSamplePoint::Init(string line)
{
  istringstream ss(line);
  ss >> fValue >> fRMS >> fWeightSum;
}

void 
STSamplePoint::Update(Double_t v, Double_t w)
{
  fValue = (fWeightSum * fValue + w * v) / (fWeightSum + w);
  fRMS = (fWeightSum * fRMS) / (fWeightSum + w)
         + w * (fValue - v) * (fValue - v) / fWeightSum;

  fWeightSum += w;
}

TString
STSamplePoint::GetSummary()
{
  return Form("%.6g %.6g %.6g", fValue, fRMS, fWeightSum);
}
