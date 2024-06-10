TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;

STVertex *vertexPtr;
STRecoTrack *recoPtr;

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

const double pi = 3.1415926;

void Diagnostics() {
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

    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    TH1D *vtxHistX = new TH1D("vtxHistX", "vtxHistX", 100, -100, 100);
    TH1D *vtxHistY = new TH1D("vtxHistY", "vtxHistY", 100, -250, -200);
    TH1D *vtxHistZ = new TH1D("vtxHistZ", "vtxHistZ", 100, -100, 100);

    TH2D *pidHist = new TH2D("pidHist", "pidHist", 1000, -1000, 2000, 1000, 0, 1000);
    TH1I *multiplicity = new TH1I("multiplicity", "multiplicity", 100, 0, 100);
    TH2D *katanaPos = new TH2D("katanaPos", "katanaPos", 130, -260, 260, 50, -15, 15);

    TH2D *thetPhi = new TH2D("thetPhi", "thetPhi", 100, 0, 90, 100, 0, 360);

    int eventCount = tree->GetEntries();

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        auto trackCount = recoReader->Get()->GetEntries();
        multiplicity->Fill(trackCount);
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

                auto pos = recoPtr->GetPosKatana();
                katanaPos->Fill(pos.X(), pos.Y());
                //cout << "track: " << r << "; mom: " << mom << "; dEdx: " << dedx << endl;

                auto momTar = recoPtr->GetMomentumTargetPlane();

                auto theta = momTar.Theta();
                auto phi = momTar.Phi();

                thetPhi->Fill(theta / pi * 180, (phi + pi) / pi * 180);
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

    TCanvas *cMul = new TCanvas("cMul", "cMul", 1);
    multiplicity->Draw();

    TCanvas *cKat = new TCanvas("cKat", "cKat", 1);
    katanaPos->Draw("COLZ");

    TCanvas *cThP = new TCanvas("cThP", "cThP", 1);
    thetPhi->Draw("COLZ");

    //TFile *outFile = new TFile(outFileName, "RECREATE");
    //pidHist->Write();
    //outFile->Close();



}
