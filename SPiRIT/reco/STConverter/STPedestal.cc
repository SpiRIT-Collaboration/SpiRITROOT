// =================================================
//  STPedestal Class
// 
//  Description:
//    This class is used for calculating or finding
//    pedestal value and sigma corresponding to
//    user-input coboIdx, asadIdx, agetIdx & chIdx.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 14
// =================================================

#include "STPedestal.hh"

#include "Riostream.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"

ClassImp(STPedestal);

STPedestal::STPedestal() {
  Initialize();
}

STPedestal::STPedestal(TString pedestalData) {
  Initialize();

  SetPedestalData(pedestalData);
}

void STPedestal::Initialize()
{
  fOpenFile = NULL;
  fPedestalTree = NULL;

  memset(fPedestal, 0, sizeof(Double_t)*512);;
  memset(fPedestalSigma, 0, sizeof(Double_t)*512);;
}

Bool_t STPedestal::SetPedestalData(TString pedestalData) {
  if (fOpenFile != NULL)
    delete fOpenFile;

  if ((fOpenFile = new TFile(pedestalData))) {
    fPedestalTree = (TTree *) fOpenFile -> Get("pedestal");
    fPedestalTree -> SetBranchAddress("pedestal", fPedestal);
    fPedestalTree -> SetBranchAddress("pedestalSigma", fPedestalSigma);

    return 1;
  }

  return 0;
}

/*
void STPedestal::GetPedestal(Int_t *samples, Double_t *pedestalArray, Int_t startIdx, Int_t numPedestalSamples) {
  Initialize();

  Int_t numSkips = 0;
  
  for (Int_t i = startIdx; i < startIdx + numPedestalSamples; i++) {
    if (samples[i] == 0) {
      numSkips++;
      continue;
    }

    pedestal += samples[i];
  }

  pedestal /= (Double_t) numPedestalSamples;

  for (Int_t i = startIdx; i < startIdx + numPedestalSamples + numSkips; i++) {
    if (samples[i] == 0)
      continue;

    pedestalSigma += TMath::Power(pedestal - (Double_t) samples[i], 2);
  }

  pedestalArray[0] = pedestal;
  pedestalArray[1] = TMath::Sqrt(pedestalSigma/(Double_t)numPedestalSamples);

  return;
}
*/

void STPedestal::GetPedestal(Int_t coboIdx, Int_t asadIdx, Int_t agetIdx, Int_t chIdx, Double_t *pedestal, Double_t *pedestalSigma) {
  if (fOpenFile == NULL) {
    cerr << "Pedestal data file is not set!" << endl;

    return;
  }
  // To one CoBo, connected are 4 AsAds, one of which has 4 AGETs composed of 68 channels
  Int_t pedestalIndex = coboIdx*(68*4*4) + asadIdx*(68*4) + agetIdx*68 + chIdx;

  fPedestalTree -> GetEntry(pedestalIndex);

  memcpy(pedestal, fPedestal, sizeof(fPedestal));
  memcpy(pedestalSigma, fPedestalSigma, sizeof(fPedestalSigma));

  return;
}
