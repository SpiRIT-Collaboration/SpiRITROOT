TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;

STVertex *vertexPtr;
STRecoTrack *recoPtr;

//Vertex Z parameters
double targetZpos = -21.86;
double targetZsig = 2.15;

//Cut on number of clusters for a given track
Int_t fClustCut = 15;

//Cut to check track actually originates from the vertex
Double_t fPOCACut = 20;


void VertexAndPID(int runNum = 1283) {
    TString filePath = TString::Format("data/run%4d_s0*.reco.2024.root", runNum);
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    //VAVertex and VATracks use the vertex from the BDC
    //vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "VAVertex");
    //recoReader = new TTreeReaderValue<TClonesArray>(*reader, "VATracks");

    //STVertex and STRecoTrack use the vertex from the tracks
    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    TH1D *vtxHistX = new TH1D("vtxHistX", "vtxHistX", 100, -25, 25);
    TH1D *vtxHistY = new TH1D("vtxHistY", "vtxHistY", 100, -250, -200);
    TH1D *vtxHistZ = new TH1D("vtxHistZ", "vtxHistZ", 40, -30, -10);

    TH2D *pidHist = new TH2D("pidHist", "pidHist", 1000, -1000, 2000, 1000, 0, 1000);

    int eventCount = tree->GetEntries();

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        auto trackCount = recoReader->Get()->GetEntries();
        //cout << "event: " << i << endl;
        //cout << "recoEntries: " << trackCount << endl;
        vertexPtr = dynamic_cast<STVertex *>((*vertexReader)->At(0));

        if(vertexPtr == NULL) {
            //cout << "vertexPtr is null" << endl;
        }
        if(vertexPtr != NULL) {
            auto vertex = vertexPtr->GetPos();
            //cout << "vertex: (" << vertex.X() << ", " << vertex.Y() << ", " << vertex.Z() << ")" << endl;
            if(vertex.Z() > targetZpos + 3 * targetZsig || vertex.Z() < targetZpos - 3 * targetZsig)
                continue;
            vtxHistX->Fill(vertex.X());
            vtxHistY->Fill(vertex.Y());
            vtxHistZ->Fill(vertex.Z());

            for(int r = 0; r < trackCount; r++) {
                recoPtr = dynamic_cast<STRecoTrack *>((*recoReader)->At(r));

                if(recoPtr == NULL) {
                    cout << "recoPtr is null" << endl;
                }
                if(recoPtr != NULL) {
                    if(recoPtr->GetNumLayerClusters() + recoPtr->GetNumRowClusters() > fClustCut && (recoPtr->GetPOCAVertex() - vertex).Mag() < fPOCACut) {
                        auto mom = recoPtr->GetMomentum().Mag();
                        auto dedx = recoPtr->GetdEdxWithCut(0, 0.7, 0.5);
                        auto charge = recoPtr->GetCharge();
                        auto gfCharge = recoPtr->GetGenfitCharge();

                        pidHist->Fill(mom * gfCharge, dedx);
                        //cout << "track: " << r << "; mom: " << mom << "; dEdx: " << dedx << endl;
                    }
                }
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