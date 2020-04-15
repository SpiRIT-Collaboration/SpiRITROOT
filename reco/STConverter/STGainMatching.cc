// =================================================
//  STGainMatching Class
// 
//  Description:
//    Developed for matching gain of anode 12 and 14
//    which have the low anode voltage.
//    However, the class is designed to be used for
//    the other anode parts.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2017. 01. 20
// =================================================

#include "STGainMatching.hh"
#include <sstream>

ClassImp(STGainMatching)

STGainMatching::STGainMatching() {
  ClassInit();
}

STGainMatching::STGainMatching(TString datafile) {
  ClassInit();

  SetDatafile(datafile);
}

Bool_t STGainMatching::Init() {
  if (fDatafile.EqualTo("")) {
    std::cout << "= [STGainMatching] Data file is not set specified!" << std::endl;

    fIsInitialized = kFALSE;
    return fIsInitialized;
  }

  Int_t layer, row;
  std::ifstream matchList(fDatafile.Data());
  std::string line;
  while(std::getline(matchList, line))
  {
    double col1, col2, col3;
    std::stringstream ss(line);
    ss >> col1 >> col2;
    layer = int(col1 + 0.5);
    if(ss >> col3)
    {
      row = int(col2 + 0.5);
      fRelativeGain[layer][row] = col3;
    }
    else
      for(int iRow = 0; iRow < 108; ++iRow)
        fRelativeGain[layer][iRow] = col2;
  }

  fIsInitialized = kTRUE;
  return fIsInitialized;
}

Bool_t STGainMatching::CalibrateADC(Int_t layer, Int_t row, Int_t numTbs, Double_t *adc) {
  if (!fIsInitialized) {
    std::cout << "= [STGainMatching] Class is not initialized properly!" << std::endl;

    return fIsInitialized;
  }

  for (Int_t iTb = 0; iTb < numTbs; iTb++)
    adc[iTb] = adc[iTb]*fRelativeGain[layer][row];

  return fIsInitialized;
}

void STGainMatching::ClassInit() {
  fDatafile = "";
  for (Int_t iLayer = 0; iLayer < 112; iLayer++)
    for (Int_t iRow = 0; iRow < 108; iRow++)
      fRelativeGain[iLayer][iRow] = 1;
}

void STGainMatching::SetDatafile(TString datafile) {
  fDatafile = datafile;
}
