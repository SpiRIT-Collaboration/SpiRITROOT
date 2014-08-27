//////////////////////////////////////////////////////////////////
//
// This macro reads raw pulser data and processed pedestal data
// to generate scaling factor of signal size.
// 
// It read first 100 pedestal-subtracted events of the given
// pulser data and calculate the average of maximum heights.
// After that, it requires the user input to determine the point
// of scale factor 1 and the threshold point.
//
//////////////////////////////////////////////////////////////////

//////////////////////////////////
//                              //
//        EDIT FROM HERE!       //
//                              //
//////////////////////////////////

TString fInputFile = "~/Common/data/pulser_20140821/pulser_5.0V.graw";
TString fPedestalFile = "pedestal.root";
TString fOutputFile = "gainCalibration.root";
TString parameterDir = "../parameters";

// In order to make sure for TSpectrum class to find the right peak,
// limit the searching range in between two values below.
Int_t peakRange[2] = {150, 300};

//////////////////////////////////
//                              //
//       EDIT UNTIL HERE!       //
//                              //
// DON'T TOUCH BELOW THIS LINE! //
//                              //
//////////////////////////////////

const Int_t fRows = 108;
const Int_t fLayers = 112;

void makeGainCalibration() {
  gSystem -> Load("libSTReco");
  gSystem -> Load("libSTFormat");

  Int_t fNumTbs = GetNumTbs();
  Int_t fPadRow, fPadLayer;

  STCore *core = new STCore(fInputFile, fNumTbs);
  TString uaMapFile = parameterDir;
  uaMapFile.Append("/UnitAsAd.map");
  TString agetMapFile = parameterDir;
  agetMapFile.Append("/AGET.map");

  core -> SetUAMap(uaMapFile);
  core -> SetAGETMap(agetMapFile);
  core -> SetPedestalData(fPedestalFile);

  TFile *file = new TFile(fOutputFile, "RECREATE");
  
  Double_t fScaleFactor = 1;
  TTree *tree = new TTree("GainCalibrationData", "Gain Calibration Data Tree");
  tree -> Branch("padRow", &fPadRow, "padRow/I");
  tree -> Branch("padLayer", &fPadLayer, "padLayer/I");
  tree -> Branch("scaleFactor", &fScaleFactor, "scaleFactor/D");

  GETMath *math[fRows][fLayers];
  for (Int_t fPadRow = 0; fPadRow < fRows; fPadRow++)
    for (Int_t fPadLayer = 0; fPadLayer < fLayers; fPadLayer++)
      math[fPadRow][fPadLayer] = new GETMath();

  TSpectrum *peakFinder = new TSpectrum(5);
  Float_t *adcTemp = new Float_t[512];
  Float_t *dummy = new Float_t[512];

  for (Int_t iEvent = 0; iEvent < 100; iEvent++) {
  STRawEvent *event = core -> GetRawEvent();

    cout << "Start event: " << event -> GetEventID() << endl;

    Int_t numPads = event -> GetNumPads();
    for (Int_t iPad = 0; iPad < numPads; iPad++) {
      STPad *pad = event -> GetPad(iPad);
      Double_t *adc = pad -> GetADC();

      for (Int_t i = 0; i < fNumTbs; i++)
        adcTemp[i] = adc[i];

      Int_t row = pad -> GetRow();
      Int_t layer = pad -> GetLayer();

      Int_t numPeaks = peakFinder -> SearchHighRes(adcTemp + peakRange[0], dummy, peakRange[1] - peakRange[2], 10, 90, kFALSE, 3, kTRUE, 3);

      if (numPeaks != 1)
        cout << row << " " << layer << " " << numPeaks << " " << (Int_t)ceil((peakFinder -> GetPositionX())[0] + peakRange[0]) << " " << adc[(Int_t)ceil((peakFinder -> GetPositionX())[0] + peakRange[0])] << endl;
      Double_t max = adc[(Int_t)ceil((peakFinder -> GetPositionX())[0] + peakRange[0])];

      math[row][layer] -> Add(max);
    }

    cout << "Done event: " << event -> GetEventID() << endl;
  }

  TCanvas c1;
  TH1D *hist = new TH1D("hist", "", 2048, 0, 4096);
  cout << "== Drawing the pulse heights distribution" << endl;
  for (Int_t fPadRow = 0; fPadRow < fRows; fPadRow++) {
    for (Int_t fPadLayer = 0; fPadLayer < fLayers; fPadLayer++) {
      fScaleFactor = math[fPadRow][fPadLayer] -> GetMean();

      if (fScaleFactor == 0) continue;
      hist -> Fill(fScaleFactor);
    }
  }
  hist -> Draw();
  c1.Update();

  Double_t center = 0;
  Double_t cut = 0;
  cout << "== Enter the central value that will be regarded as scale factor 1: ";
  cin >> center;
  cout << "== Enter the cut value that the pads below the value will be ignored: ";
  cin >> cut;

  cout << "== Creating gain calibration data: " << fOutputFile << endl;
  for (Int_t fPadRow = 0; fPadRow < fRows; fPadRow++) {
    for (Int_t fPadLayer = 0; fPadLayer < fLayers; fPadLayer++) {
      fScaleFactor = (math[fPadRow][fPadLayer] -> GetMean() < cut ? 0 : center/(math[fPadRow][fPadLayer] -> GetMean()));

      tree -> Fill();
    }
  }

  delete hist;
  file -> Write();
  cout << "== Gain calibration data " << fOutputFile << " Created!" << endl;
}

Int_t GetNumTbs() {
  TString parameterFile = parameterDir;
  parameterFile.Append("/ST.parameters.par");
  ifstream parameters(parameterFile);
  while (kTRUE) {
    TString numTbs;
    numTbs.ReadToken(parameters);
    if (numTbs.EqualTo("NumTbs:Int_t")) {
      numTbs.ReadToken(parameters);
      parameters.close();
      return numTbs.Atoi();
    }
  }
}
