TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;

STVertex *vertexPtr;
STRecoTrack *recoPtr;

void VertexAndPID(int runNum = 3154) {
    TString filePath = TString::Format("data/run%4d_s0.reco.test.root", runNum);
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    TH1D *vtxHistX = new TH1D("vtxHistX", "vtxHistX", 100, -25, 25);
    TH1D *vtxHistY = new TH1D("vtxHistY", "vtxHistY", 100, -250, -200);
    TH1D *vtxHistZ = new TH1D("vtxHistZ", "vtxHistZ", 20, -20, -10);

    TH2D *pidHist = new TH2D("pidHist", "pidHist", 1000, -1000, 2000, 1000, 0, 1000);

    int eventCount = tree->GetEntries();

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        auto trackCount = recoReader->Get()->GetEntries();
        cout << "event: " << i << endl;
        cout << "recoEntries: " << trackCount << endl;
        vertexPtr = dynamic_cast<STVertex *>((*vertexReader)->At(0));

        if(vertexPtr == NULL) {
            cout << "vertexPtr is null" << endl;
        }
        if(vertexPtr != NULL) {
            auto vertex = vertexPtr->GetPos();
            cout << "vertex: (" << vertex.X() << ", " << vertex.Y() << ", " << vertex.Z() << ")" << endl;
            vtxHistX->Fill(vertex.X());
            vtxHistY->Fill(vertex.Y());
            vtxHistZ->Fill(vertex.Z());
        }

        for(int r = 0; r < trackCount; r++) {
            recoPtr = dynamic_cast<STRecoTrack *>((*recoReader)->At(r));

            if(recoPtr == NULL) {
                cout << "recoPtr is null" << endl;
            }
            if(recoPtr != NULL) {
                auto mom = recoPtr->GetMomentum().Mag();
                auto dedx = recoPtr->GetdEdxWithCut(0, 0.7, 0.5);

                pidHist->Fill(mom, dedx);
                //cout << "track: " << r << "; mom: " << mom << "; dEdx: " << dedx << endl;
            }
        }
    }

    TCanvas *cVX = new TCanvas("cVX", "cVX", 1);
    vtxHistX->Draw();

    TCanvas *cVY = new TCanvas("cVY", "cVY", 1);
    vtxHistY->Draw();

    TCanvas *cVZ = new TCanvas("cVZ", "cVZ", 1);
    vtxHistZ->Draw();

    TCanvas *cPID = new TCanvas("cPID", "cPID", 1);
    pidHist->Draw("COLZ");




}