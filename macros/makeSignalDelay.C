//////////////////////////////////////////////////////////////////
//
// This macro reads raw pulser data and processed pedestal data
// to generate delaying factor for each AsAd board.
// 
// It read first 100 pedestal-subtracted events of the given
// pulser data and calculate the time at which the charge arrives
// on pads. After that, it requires the user input to determine
// the zero time point. 
//
//////////////////////////////////////////////////////////////////

//////////////////////////////////
//                              //
//        EDIT FROM HERE!       //
//                              //
//////////////////////////////////

TString fInputFile = "~/Common/data/pulser_20140821/pulser_5.0V.graw";
TString fPedestalFile = "pedestal.root";
TString fOutputFile = "signalDelay.root";
TString parameterDir = "../parameters";

//////////////////////////////////
//                              //
//       EDIT UNTIL HERE!       //
//                              //
// DON'T TOUCH BELOW THIS LINE! //
//                              //
//////////////////////////////////

const Int_t fUARows = 12;
const Int_t fUALayers = 4;

void makeSignalDelay() {
  gSystem -> Load("libSTReco");
  gSystem -> Load("libSTFormat");

  Int_t fNumTbs = GetNumTbs();
  Int_t fPadRow, fPadLayer;
  Int_t fUARow, fUALayer, fUAIdx;

  STCore *core = new STCore(fInputFile, fNumTbs);
  TString uaMapFile = parameterDir;
  uaMapFile.Append("/UnitAsAd.map");
  TString agetMapFile = parameterDir;
  agetMapFile.Append("/AGET.map");

  core -> SetUAMap(uaMapFile);
  core -> SetAGETMap(agetMapFile);
  core -> SetPedestalData(fPedestalFile);

  TFile *file = new TFile(fOutputFile, "RECREATE");
  
  Double_t fSignalDelay = fNumTbs;
  TTree *tree = new TTree("SignalDelayData", "Signal Delay Data Tree");
  tree -> Branch("UAIdx", &fUAIdx, "UAIdx/I");
  tree -> Branch("signalDelay", &fSignalDelay, "signalDelay/D");

  GETMath *math[fUALayers][fUARows];
  for (Int_t fUALayer = 0; fUALayer < fUALayers; fUALayer++)
    for (Int_t fUARow = 0; fUARow < fUARows; fUARow++)
        math[fUALayer][fUARow] = new GETMath();

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

      Int_t numPeaks = peakFinder -> SearchHighRes(adcTemp, dummy, fNumTbs, 4.7, 90, kFALSE, 3, kTRUE, 3);

      if (numPeaks != 1)
        cout << row << " " << layer << " " << numPeaks << " " << (Int_t)ceil((peakFinder -> GetPositionX())[0]) << " " << adc[(Int_t)ceil((peakFinder -> GetPositionX())[0])] << endl;

      math[(layer/28)][row/9] -> Add(peakFinder -> GetPositionX()[0]);
    }

    cout << "Done event: " << event -> GetEventID() << endl;
  }

  gROOT -> SetStyle("2d")
  gStyle -> SetOptStat(0);
  TCanvas c1;
  c1 -> SetGrid();
  TH2D *hist = new TH2D("hist", "", 4, 0, 4, 12, 0, 12);
  hist -> GetXaxis() -> SetNdivisions(4);
  hist -> GetYaxis() -> SetNdivisions(12);
  cout << "== Drawing the pulse heights distribution" << endl;
  for (Int_t fUALayer = 0; fUALayer < fUALayers; fUALayer++) {
    for (Int_t fUARow = 0; fUARow < fUARows; fUARow++) {
      fSignalDelay = math[fUALayer][fUARow] -> GetMean();

      if (fSignalDelay == 0) continue;
      hist -> Fill(fUALayer, fUARow, fSignalDelay);
    }
  }
  hist -> SetMarkerSize(1.5);
  hist -> Draw("text colz");
  c1.Update();

  Int_t zero = 0;
  cout << "== Enter the zero AsAd UA index that will be regarded as signal delay 0: ";
  cin >> zero;

  Double_t zeroPoint = math[zero/100][zero%100] -> GetMean();

  cout << "== Creating gain calibration data: " << fOutputFile << endl;
  for (Int_t fUALayer = 0; fUALayer < fUALayers; fUALayer++) {
    for (Int_t fUARow = 0; fUARow < fUARows; fUARow++) {
      fSignalDelay = zeroPoint - math[fUALayer][fUARow] -> GetMean();
      fUAIdx = fUALayer*100 + fUARow;

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
