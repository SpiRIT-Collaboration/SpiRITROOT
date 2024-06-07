TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;

STVertex *vertexPtr;
STRecoTrack *recoPtr;

void Diagnostics(int runNum = 63) {
    TString outFileName = "PIDwoGG.root";
    //TString filePath = TString::Format("data/run%04d_s0.reco.test.root", runNum);
    //TString filePath = "data/multi01OnlyA.root";
    tree = new TChain("cbmsim");

    tree->Add("data/run_64.root");
    tree->Add("data/run_65.root");
    tree->Add("data/run_72.root");
    tree->Add("data/run_73.root");
    tree->Add("data/run_74.root");
    tree->Add("data/run_75.root");
    tree->Add("data/run_96.root");
    tree->Add("data/run_97.root");
    tree->Add("data/run_98.root");

    //tree->Add("data/run_92.root");
    //tree->Add("data/run_93.root");
    //tree->Add("data/run_94.root");
    //tree->Add("data/run_95.root");

    //tree->Add("data/multi10Less.root");
    //tree->Add("data/multi10Less_1.root");


    reader = new TTreeReader(tree);

    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    TH1D *vtxHistX = new TH1D("vtxHistX", "vtxHistX", 100, -100, 100);
    TH1D *vtxHistY = new TH1D("vtxHistY", "vtxHistY", 100, -250, -200);
    TH1D *vtxHistZ = new TH1D("vtxHistZ", "vtxHistZ", 100, -100, 100);

    int nBinsX = 100;
    double xMin = 1;     // Starting value (log base 10 of this should be an integer if you want nice labels)
    double xMax = 100000;  // Ending value (log base 10 of this should be an integer if you want nice labels)

// Calculate the bin edges
    std::vector<double> xBins(nBinsX + 1);
    double logMin = std::log10(xMin);
    double logMax = std::log10(xMax);
    double binWidth = (logMax - logMin) / nBinsX;

    for (int i = 0; i <= nBinsX; ++i) {
       xBins[i] = std::pow(10, logMin + i * binWidth);
    }

    TH2D *pidHist = new TH2D("pidHist", "pidHist", nBinsX, xBins.data(), 100, 0, 350);
    TH1D *multiplicity = new TH1I("multiplicity", "multiplicity", 50, 0, 50);
    TH2D *katanaPos = new TH2D("katanaPos", "katanaPos", 130, -260, 260, 50, -15, 15);

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

    TCanvas *cKat = new Tacanvas("cKat", "cKat", 1);
    katanaPos->Draw("COLZ");

    //TFile *outFile = new TFile(outFileName, "RECREATE");
    //pidHist->Write();
    //outFile->Close();



}
