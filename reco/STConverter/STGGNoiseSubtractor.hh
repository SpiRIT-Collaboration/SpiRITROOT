// =================================================
//  STGGNoiseSubtractor Class
// 
//  Description:
//    Subtract GG Noise using pre-prepared data
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2016. 04. 04
// =================================================

#ifndef STGGNOISESUBTRACTOR
#define STGGNOISESUBTRACTOR

#include "TFile.h"
#include "TString.h"

#include "GETMath.hh"

class STGGNoiseSubtractor {
  public:
    STGGNoiseSubtractor();
    STGGNoiseSubtractor(TString ggNoiseData, Int_t numTbs = 512, Double_t rmsCut = 5);
    ~STGGNoiseSubtractor() {};

    Bool_t Init();

    void SetGGNoiseData(TString ggNoiseData);
    void SetNumTbs(Int_t numTbs);
    void SetRMSCut(Double_t rmsCut);

    Bool_t IsSetGGNoiseData();
    Bool_t SubtractNoise(Int_t row, Int_t layer, Double_t *adc);

  private:
    TString fGGNoiseFile;
    TFile *fOpenFile;

    GETMath *fMath;

    Double_t ***fNoise;
    Double_t **fMean;

    Double_t fRmsCut;
    Int_t fNumTbs;

    Bool_t fIsGGNoiseData;

  ClassDef(STGGNoiseSubtractor, 1)
};

#endif
