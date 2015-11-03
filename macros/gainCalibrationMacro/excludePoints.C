/**
  * This macro generated new gain calibration data by excluding specified points.
  * 
  * Caution: Generating new checking data takes really loooooooooong time. (about 30 mins.)
  *          So, at first run this macro without generating it and later do it.
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Input data file. You should put checking data file here.
TString fCheckingDataFile = "gainCalibration_groundPulsing_20151029.checking.root";

// Set the output data file.
TString fOutputDataFile = "gainCalibration_groundPulsing_20151029.root.excluded";

// Number of voltages in the data
Int_t fNumPoints = 24;

// The numbers of excluding points
Int_t fExcludingPoints[] = {21, 22, 23};

// If set kTRUE, this will generate checking dat
Bool_t fGenerateCheckingDataFile = kFALSE;

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

Int_t fNumExcludingPoints = sizeof(fExcludingPoints)/sizeof(Int_t);
Int_t fNewNumPoints = fNumPoints - fNumExcludingPoints;

void excludePoints() {
  sort(fExcludingPoints, fExcludingPoints + fNumExcludingPoints, std::greater<Int_t>());

  Int_t padRow, padLayer;
  Double_t constant, linear, quadratic;

  TFile *fOpenCheckingDataFile = new TFile(fCheckingDataFile);

  TFile *fWriteDataFile = new TFile(fOutputDataFile, "recreate");
  TTree *fWriteTree = new TTree("GainCalibrationData", "Gain Calibration Data Tree");
  fWriteTree -> Branch("padRow", &padRow);
  fWriteTree -> Branch("padLayer", &padLayer);
  fWriteTree -> Branch("constant", &constant);
  fWriteTree -> Branch("linear", &linear);
  fWriteTree -> Branch("quadratic", &quadratic);

  TFile *fWriteCheckingDataFile;
  if (fGenerateCheckingDataFile) { 
    fOutputDataFile.ReplaceAll(".root", ".checking.root");
    fWriteCheckingDataFile = new TFile(fOutputDataFile, "recreate");
  }

  TIter next(fOpenCheckingDataFile -> GetListOfKeys());
  TKey *key;
  while ((key = (TKey *) next())) {
    TString name = key -> GetName();
    TObjArray *nameArray = name.Tokenize("_");
    TString type = ((TObjString *) nameArray -> At(0)) -> GetString();

    Int_t row = ((TObjString *) nameArray -> At(1)) -> GetString().Atoi();
    Int_t layer = ((TObjString *) nameArray -> At(2)) -> GetString().Atoi();

    if (type.EqualTo("pad")) {
      TGraph *graph = (TGraph *) fOpenCheckingDataFile -> Get(name);

      for (Int_t iPoint : fExcludingPoints)
        graph -> RemovePoint(iPoint);

      graph -> Fit("pol2", "Q");
      TF1 *fit = ((TF1 *) graph -> GetFunction("pol2"));

      padRow = row;
      padLayer = layer;
      constant = fit -> GetParameter(0);
      linear = fit -> GetParameter(1);
      quadratic = fit -> GetParameter(2);

      fWriteTree -> Fill();

      if (fGenerateCheckingDataFile) {
        fWriteCheckingDataFile -> cd();
        graph -> Write();
      }
    } else {
      if (!fGenerateCheckingDataFile)
        break;

      Int_t voltage = ((TObjString *) nameArray -> At(3)) -> GetString().Atoi();

      Bool_t check = kFALSE;
      for (Int_t test : fExcludingPoints) {
        if (voltage == test) {
          check = kTRUE;
          break;
        }
      }

      if (check)
        continue;

      TH1D *hist = (TH1D *) fOpenCheckingDataFile -> Get(name);
      fWriteCheckingDataFile -> cd();
      hist -> Write();
    }
  }

  delete fOpenCheckingDataFile;
  if (fGenerateCheckingDataFile)
    delete fWriteCheckingDataFile;

  fWriteDataFile -> cd();
  fWriteTree -> Write();

  delete fWriteDataFile;
}
