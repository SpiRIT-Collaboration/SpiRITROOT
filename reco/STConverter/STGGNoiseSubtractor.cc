// =================================================
//  STGGNoiseSubtractor Class
// 
//  Description:
//    Subtract GG Noise using pre-prepared data
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2016. 04. 04
// =================================================

#include "STGGNoiseSubtractor.hh"

#include <algorithm>
#include <iostream>

#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"

ClassImp(STGGNoiseSubtractor)

STGGNoiseSubtractor::STGGNoiseSubtractor()
{
  fGGNoiseFile = "";
  fOpenFile = nullptr;

  fNoise = new Double_t**[112];
  fMean = new Double_t*[108];
  for (Int_t iRow = 0; iRow < 108; iRow++) {
    fNoise[iRow] = new Double_t*[112];
    fMean[iRow] = new Double_t[112];
    std::fill_n(fMean[iRow], 112, 0);
    for (Int_t iLayer = 0; iLayer < 112; iLayer++) {
      fNoise[iRow][iLayer] = new Double_t[512];
      std::fill_n(fNoise[iRow][iLayer], 512, 0);
    }
  }

  fMath = new GETMath();
}

STGGNoiseSubtractor::STGGNoiseSubtractor(TString ggNoiseData, Int_t numTbs, Double_t rmsCut)
:STGGNoiseSubtractor()
{
  SetGGNoiseData(fGGNoiseFile);
  SetNumTbs(numTbs);
  SetRMSCut(rmsCut);
  Init();
}

Bool_t STGGNoiseSubtractor::Init()
{
  fOpenFile = new TFile(fGGNoiseFile);

  if (fOpenFile -> FindObjectAny("GatingGridNoise") == nullptr) {
    fOpenFile -> Close();
    delete fOpenFile;
    fOpenFile = nullptr;

    std::cout << "[STGGNoiseSubtractor] GatingGridNoise tree doesn't exist!" << std::endl;

    return kFALSE;
  }

  TTreeReader reader("GatingGridNoise", fOpenFile);
  TTreeReaderValue<Int_t> row(reader, "row");
  TTreeReaderValue<Int_t> layer(reader, "layer");
  TTreeReaderArray<Double_t> noise(reader, "noise");

  Int_t startTb = 1;
  Int_t averageTbs = 10;

  while (reader.Next()) {
    for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
      fNoise[*row][*layer][iTb] = noise[iTb];

    startTb = 1;

    while (1) {
      fMath -> Reset();
      for (Int_t iTb = startTb; iTb < startTb + averageTbs; iTb++)
        fMath -> Add(fNoise[*row][*layer][iTb]);

      if (fMath -> GetRMS() < fRmsCut)
        break;

      startTb++;

      if (startTb > fNumTbs - averageTbs - 2) {
        std::cout << "= [STGGNoiseSubtractor] There's no part in (row=" << *row << ",layer=";
        std::cout << *layer << " noise satisfying sigma threshold " << fRmsCut << "!" << std::endl;

        fIsGGNoiseData = kFALSE;
        return IsSetGGNoiseData();
      }
    }

    fMean[*row][*layer] = fMath -> GetMean();
  }

  fIsGGNoiseData = kTRUE;
  return IsSetGGNoiseData();
}

  void STGGNoiseSubtractor::SetGGNoiseData(TString ggNoiseData) { fGGNoiseFile = ggNoiseData; }
  void STGGNoiseSubtractor::SetNumTbs(Int_t numTbs)             { fNumTbs = numTbs; }
  void STGGNoiseSubtractor::SetRMSCut(Double_t rmsCut)          { fRmsCut = rmsCut; }

Bool_t STGGNoiseSubtractor::IsSetGGNoiseData()                  { return fIsGGNoiseData; }

Bool_t STGGNoiseSubtractor::SubtractNoise(Int_t row, Int_t layer, Double_t *adc)
{
  Int_t startTb = 1;
  Int_t averageTbs = 10;

  while (1) {
    fMath -> Reset();
    for (Int_t iTb = startTb; iTb < startTb + averageTbs; iTb++)
      fMath -> Add(adc[iTb]);

    if (fMath -> GetRMS() < fRmsCut)
      break;

    startTb++;

    if (startTb > fNumTbs - averageTbs - 2) {
      std::cout << "= [STGGNoiseSubtractor] There's no part in (row=" << row << ",layer=";
      std::cout << layer << " signal satisfying sigma threshold " << fRmsCut << "!" << std::endl;

      return kFALSE;
    }
  }

  Double_t baselineDiff = fMath -> GetMean() - fMean[row][layer];

  for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
    adc[iTb] -= baselineDiff + fNoise[row][layer][iTb];

  return kTRUE;
}
