TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;
TTreeReaderValue<TClonesArray> *recoVAReader = nullptr;

STVertex *vertexPtr;
STRecoTrack *recoPtr;
STRecoTrack *recoVAPtr;

//Vertex Z parameters
double targetZpos = -21.86;
//double targetZpos = -32.8;
//double targetZsig = 2.15;
double targetZsig = 2.5;

//Vertex X parameters
double targetXwidth = 30;

//Cut on number of clusters for a given track
//Int_t fClustCut = 15;

//Cut to check track actually originates from the vertex
//Double_t fPOCACut = 20;

//Cut on multiplicity
//Int_t fMultiCut = 15;


void SimTest(int runNum = 1234) {
    //TString filePath = TString::Format("data_sim/_s0.layer_cut.112.reco.v1.04.root", runNum, densNum);
    TString filePath = TString::Format("data_sim/_s0.layer_cut.112.reco.v1.04.root");
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    //VAVertex and VATracks use the vertex from the BDC
    //vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "VAVertex");
    recoVAReader = new TTreeReaderValue<TClonesArray>(*reader, "VATracks");

    //STVertex and STRecoTrack use the vertex from the tracks
    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    TH1D *momHist = new TH1D("momHist", "momHist", 1000, 0, 2000);
    TH1D *tgtXhist = new TH1D("tgtXhist", "tgtXhist", 50, -25, 25);
    TH1D *tgtYhist = new TH1D("tgtYhist", "tgtYhist", 50, -190 - 25, -190 + 25);

    int eventCount = tree->GetEntries();

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        auto trackCount = recoReader->Get()->GetEntries();
        auto trackVACount = recoVAReader->Get()->GetEntries();
        //cout << "event: " << i << endl;
        //cout << "recoEntries: " << trackCount << endl;
        vertexPtr = dynamic_cast<STVertex *>((*vertexReader)->At(0));

        for(int r = 0; r < trackCount; r++) {
            recoPtr = dynamic_cast<STRecoTrack *>((*recoReader)->At(r));

            if(recoPtr == NULL) {
                cout << "recoPtr is null" << endl;
                continue;
            }
            if(1) {
                auto mom = recoPtr->GetMomentum().Mag();
                auto posTgt =  recoPtr->GetPosTargetPlane();

                momHist->Fill(mom);
                tgtXhist->Fill(posTgt.X());
                tgtYhist->Fill(posTgt.Y());
                cout << "Target X: " << posTgt.X() << "; Target Y: " << posTgt.Y() << "; Momentum: " << mom << endl;
            }
        }
    }

    TString outFilePath = TString::Format("SIMtest/simTest.root");


    TFile *outFile = new TFile(outFilePath.Data(), "RECREATE");
    momHist->Write();
    tgtXhist->Write();
    tgtYhist->Write();
}