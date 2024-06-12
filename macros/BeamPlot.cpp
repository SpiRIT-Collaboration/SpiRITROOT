TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *rawEventReader = nullptr;

STRawEvent *rawEventPtr;

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

const double pi = 3.1415926;

void BeamPlot() {
  if (!(gSystem -> Getenv("RUN"))) {
    cout << endl;
    cout << cYELLOW << "== Usage: " << cNORMAL << "RUN=" << cRED << "####" << cNORMAL << " root Diagnostic.cpp" << endl;
    cout << endl;
    gSystem -> Exit(0);
  }

  Int_t runNum = atoi(gSystem -> Getenv("RUN"));

    TString filePath = TString::Format("data/run%04d_s0.reco.test.root", runNum);
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    rawEventReader = new TTreeReaderValue<TClonesArray>(*reader, "STRawEvent");

    TH2D *beamHist = new TH2D("beamHist", "beamHist", 112, 0, 1344, 108, -432, 432);

    int eventCount = tree->GetEntries();

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        cout << "event: " << i << endl;
        rawEventPtr = dynamic_cast<STRawEvent *>((*rawEventReader)->At(0));

        if(rawEventPtr == NULL) {
            cout << "rawEventPtr is null" << endl;
        }
        if(rawEventPtr != NULL) {
            for(int layer = 0; layer < 112; layer++) {
                for(int row = 0; row < 108; row++) {
                    auto pad = rawEventPtr->GetPad(row, layer);
                    auto adc = pad->GetADC();
                    double maxADC = 0;
                    for(int r = 0; r < 512; r++) {
                        if(pad->GetADC(r) > maxADC)
                            maxADC = pad->GetADC(r);
                    }

                    auto currVal = beamHist->GetBinContent(layer + 1, row + 1);
                    beamHist->SetBinContent(layer + 1, row + 1, currVal + maxADC);
                }
            }
        }
    }

    beamHist->Draw("COLZ");
}
