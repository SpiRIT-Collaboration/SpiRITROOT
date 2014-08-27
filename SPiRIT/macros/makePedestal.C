//////////////////////////////////////////////////////////////////
//
// This macro reads raw pedestal data to generate processed
// pedestal data used as an input to analyze experimental data.
//
//////////////////////////////////////////////////////////////////

//////////////////////////////////
//                              //
//        EDIT FROM HERE!       //
//                              //
//////////////////////////////////

TString fInputFile = "~/Common/data/pulser_20140821/pedestal.graw";
TString fOutputFile = "pedestal.root";
TString parameterDir = "../parameters";

Int_t fEventList[] = {0, 1, 2, 3, 7, 8, 11, 13, 14, 15, 18, 19, 20, 22, 23};

// if fNumEvents is 0, all events in the fInputFile are processed to obtain pedestal data.
//const Int_t fNumEvents = 0;
const Int_t fNumEvents = sizeof(fEventList)/sizeof(Int_t);

//////////////////////////////////
//                              //
//       EDIT UNTIL HERE!       //
//                              //
// DON'T TOUCH BELOW THIS LINE! //
//                              //
//////////////////////////////////


const Int_t fRows = 108;
const Int_t fLayers = 112;

void makePedestal() {
  CreatePedestal(fNumEvents, fEventList);
}

void CreatePedestal(Int_t numEvents, Int_t *eventList) {
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

  TFile *file = new TFile(fOutputFile, "RECREATE");
  
  Double_t fPedestal[2][512] = {{0}};
  TTree *tree = new TTree("PedestalData", "Pedestal Data Tree");
  tree -> Branch("padRow", &fPadRow, "padRow/I");
  tree -> Branch("padLayer", &fPadLayer, "padLayer/I");
  tree -> Branch("pedestal", &fPedestal[0], Form("pedestal[%d]/D", fNumTbs));
  tree -> Branch("pedestalSigma", &fPedestal[1], Form("pedestalSigma[%d]/D", fNumTbs));

  GETMath *math[fRows][fLayers][512];
  for (Int_t fPadRow = 0; fPadRow < fRows; fPadRow++)
    for (Int_t fPadLayer = 0; fPadLayer < fLayers; fPadLayer++)
      for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
        math[fPadRow][fPadLayer][iTb] = new GETMath();

  STRawEvent *event = NULL;
  Int_t eventID = 0;
  while ((event = core -> GetRawEvent())) {
    if (numEvents == 0) {}
    else if (eventID == numEvents)
      break;
    else if (eventList[eventID] != event -> GetEventID())
      continue;

    cout << "Start event: " << event -> GetEventID() << endl;

    Int_t numPads = event -> GetNumPads();
    for (Int_t iPad = 0; iPad < numPads; iPad++) {
      STPad *pad = event -> GetPad(iPad);
      Int_t *adc = pad -> GetRawADC();

      Int_t row = pad -> GetRow();
      Int_t layer = pad -> GetLayer();

      for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
        math[row][layer][iTb] -> Add(adc[iTb]);
    }

    cout << "Done event: " << event -> GetEventID() << endl;

    eventID++;
  }

  cout << "== Creating Pedestal data: " << fOutputFile << endl;
  for (Int_t fPadRow = 0; fPadRow < fRows; fPadRow++) {
    for (Int_t fPadLayer = 0; fPadLayer < fLayers; fPadLayer++) {
      for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
        fPedestal[0][iTb] = math[fPadRow][fPadLayer][iTb] -> GetMean();
        fPedestal[1][iTb] = math[fPadRow][fPadLayer][iTb] -> GetRMS();
      }

      tree -> Fill();
    }
  }

  file -> Write();
  cout << "== Pedestal data " << fOutputFile << " Created!" << endl;
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

