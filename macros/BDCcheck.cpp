void BDCcheck(int fRunNum = 1286){
    double fVertZPosCut = -21.3;
    double fVertZSigCut = 2.24;

    auto vtxXY = new TH2D("vtxXY", "vtxXY", 200, -50, 50, 200, -50, 50);
    vtxXY->SetTitle("Vertex position");
    vtxXY->GetXaxis()->SetTitle("X (mm)");
    vtxXY->GetXaxis()->SetTitleSize(0.05);
    vtxXY->GetXaxis()->SetLabelSize(0.05);
    vtxXY->GetXaxis()->CenterTitle();
    vtxXY->GetYaxis()->SetTitle("Y (mm)");
    vtxXY->GetYaxis()->SetTitleSize(0.05);
    vtxXY->GetYaxis()->SetLabelSize(0.05);
    vtxXY->GetYaxis()->CenterTitle();
    auto tbdcXY = new TH2D("tbdcXY", "tbdcXY", 200, -50, 50, 200, -50, 50);
    tbdcXY->SetTitle("BDC projection to target");
    tbdcXY->GetXaxis()->SetTitle("X (mm)");
    tbdcXY->GetXaxis()->SetTitleSize(0.05);
    tbdcXY->GetXaxis()->SetLabelSize(0.05);
    tbdcXY->GetXaxis()->CenterTitle();
    tbdcXY->GetYaxis()->SetTitle("Y (mm)");
    tbdcXY->GetYaxis()->SetTitleSize(0.05);
    tbdcXY->GetYaxis()->SetLabelSize(0.05);
    tbdcXY->GetYaxis()->CenterTitle();
    auto vtxVtbdcX = new TH2D("vtxVtbdcX", "vtxVtbdcX", 200, -50, 50, 200, -50, 50);
    vtxVtbdcX->SetTitle("Vertex X vs BDC projection X");
    vtxVtbdcX->GetXaxis()->SetTitle("Vertex X (mm)");
    vtxVtbdcX->GetXaxis()->SetTitleSize(0.05);
    vtxVtbdcX->GetXaxis()->SetLabelSize(0.05);
    vtxVtbdcX->GetXaxis()->CenterTitle();
    vtxVtbdcX->GetYaxis()->SetTitle("BDC X (mm)");
    vtxVtbdcX->GetYaxis()->SetTitleSize(0.05);
    vtxVtbdcX->GetYaxis()->SetLabelSize(0.05);
    vtxVtbdcX->GetYaxis()->CenterTitle();
    auto vtxVtbdcY = new TH2D("vtxVtbdcY", "vtxVtbdcY", 200, -50, 50, 200, -50, 50);
    vtxVtbdcY->SetTitle("Vertex Y vs BDC projection Y");
    vtxVtbdcY->GetXaxis()->SetTitle("Vertex Y (mm)");
    vtxVtbdcY->GetXaxis()->SetTitleSize(0.05);
    vtxVtbdcY->GetXaxis()->SetLabelSize(0.05);
    vtxVtbdcY->GetXaxis()->CenterTitle();
    vtxVtbdcY->GetYaxis()->SetTitle("BDC Y (mm)");
    vtxVtbdcY->GetYaxis()->SetTitleSize(0.05);
    vtxVtbdcY->GetYaxis()->SetLabelSize(0.05);
    vtxVtbdcY->GetYaxis()->CenterTitle();
    auto vtxHistZ = new TH1D("vtxHistZ", "vtxHistZ", 1000, -500, 800);
    vtxHistZ->SetTitle("Vertex Z Position");
    vtxHistZ->GetXaxis()->SetTitle("Vertex Z (mm)");
    //vtxHistZ->GetXaxis()->SetTitleSize(0.05);
    //vtxHistZ->GetXaxis()->SetLabelSize(0.05);
    vtxHistZ->GetXaxis()->CenterTitle();
    vtxHistZ->GetYaxis()->SetTitle("Counts");
    //vtxHistZ->GetYaxis()->SetTitleSize(0.05);
    //vtxHistZ->GetYaxis()->SetLabelSize(0.05);
    vtxHistZ->GetYaxis()->CenterTitle();

    auto vtxX = new TH1D("vtxX", "vxtX", 200, -50, 50);
    vtxX->SetTitle("Vertex X Position");
    vtxX->GetXaxis()->SetTitle("Vertex X (mm)");
    vtxX->GetXaxis()->SetTitleSize(0.05);
    vtxX->GetXaxis()->SetLabelSize(0.05);
    vtxX->GetXaxis()->CenterTitle();
    vtxX->GetYaxis()->SetTitle("Counts");
    vtxX->GetYaxis()->SetTitleSize(0.05);
    vtxX->GetYaxis()->SetLabelSize(0.05);
    vtxX->GetYaxis()->CenterTitle();
    auto vtxY = new TH1D("vtxY", "vxtY", 200, -50, 50);
    vtxY->SetTitle("Vertex Y Position");
    vtxY->GetXaxis()->SetTitle("Vertex Y (mm)");
    vtxY->GetXaxis()->SetTitleSize(0.05);
    vtxY->GetXaxis()->SetLabelSize(0.05);
    vtxY->GetXaxis()->CenterTitle();
    vtxY->GetYaxis()->SetTitle("Counts");
    vtxY->GetYaxis()->SetTitleSize(0.05);
    vtxY->GetYaxis()->SetLabelSize(0.05);
    vtxY->GetYaxis()->CenterTitle();

    auto tbdcX = new TH1D("tbdcX", "tbdcX", 200, -50, 50);
    tbdcX->SetTitle("BDC Projection X");
    tbdcX->GetXaxis()->SetTitle("BDC projection X (mm)");
    tbdcX->GetXaxis()->SetTitleSize(0.05);
    tbdcX->GetXaxis()->SetLabelSize(0.05);
    tbdcX->GetXaxis()->CenterTitle();
    tbdcX->GetYaxis()->SetTitle("Counts");
    tbdcX->GetYaxis()->SetTitleSize(0.05);
    tbdcX->GetYaxis()->SetLabelSize(0.05);
    tbdcX->GetYaxis()->CenterTitle();
    auto tbdcY = new TH1D("tbdcY", "tbdcY", 200, -50, 50);
    tbdcY->SetTitle("BDC Projection Y");
    tbdcY->GetXaxis()->SetTitle("BDC projection Y (mm)");
    tbdcY->GetXaxis()->SetTitleSize(0.05);
    tbdcY->GetXaxis()->SetLabelSize(0.05);
    tbdcY->GetXaxis()->CenterTitle();
    tbdcY->GetYaxis()->SetTitle("Counts");
    tbdcY->GetYaxis()->SetTitleSize(0.05);
    tbdcY->GetYaxis()->SetLabelSize(0.05);
    tbdcY->GetYaxis()->CenterTitle();

   TChain *tree = new TChain("cbmsim");

   TString format = "reco.2024";

   TString filePath = TString::Format("data/run%04d_s*.%s.root", fRunNum, format.Data());

   tree->Add(filePath);

    auto reader = new TTreeReader(tree);

    auto vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    auto recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");
    auto rawEventReader = new TTreeReaderValue<TClonesArray>(*reader, "STRawEvent");


    int eventCount = tree->GetEntries();


    Double_t tbdc_x;
    Double_t tbdc_y;
    Double_t tbdc_a;
    Double_t tbdc_b;
    Double_t bdc_x;
    Double_t bdc_y;
    Double_t bdc_a;
    Double_t bdc_b;

    bool useBDC = true;

    auto fBDCProjection = new STBDCProjection(TString(gSystem -> Getenv("VMCWORKDIR")) + "/parameters/ReducedBMap.txt");
    //fBDCProjection -> setBeam(runNum);

    TString fBeamData = Form("/mnt/cephfs/hira/SPIRIT_2024/beam_VETO_KATANA_data/bdc_rootfiles/bdc_%d.root", fRunNum);
    auto fBeamFile = new TFile(fBeamData);
    TTree *fBeamTree;
    if(!fBeamFile) {
       cout << "ERROR: BDC file does not exist!" << endl;
       cout << "Ignoring BDC." << endl;
       useBDC = false;
    }
    else {
       fBeamTree= (TTree *) fBeamFile -> Get("TBDC");
       if(!fBeamTree) {
         cout << "ERROR: BDC tree is missing!" << endl;
         cout << "Ignoring BDC." << endl;
         useBDC = false;
       }
       else {
         fBeamTree -> SetBranchAddress("target_x", &tbdc_x);
         fBeamTree -> SetBranchAddress("target_y", &tbdc_y);
         fBeamTree -> SetBranchAddress("target_a", &tbdc_a);
         fBeamTree -> SetBranchAddress("target_b", &tbdc_b);
         fBeamTree -> SetBranchAddress("bdc2x", &bdc_x);
         fBeamTree -> SetBranchAddress("bdc2y", &bdc_y);
         fBeamTree -> SetBranchAddress("bdcax", &bdc_a);
         fBeamTree -> SetBranchAddress("bdcby", &bdc_b);
       }
    }

    if(useBDC) {
      if(eventCount > fBeamTree->GetEntriesFast()) {
         cout << "ERROR: BDC file does not have enough events!" << endl;
         cout << "Ignoring BDC." << endl;
         useBDC = false;
      }
    }

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        if(useBDC)
           fBeamTree->GetEntry(i);

        auto vertexPtr = dynamic_cast<STVertex *>((*vertexReader)->At(0));

        TVector3 vertex;

        if(vertexPtr == NULL) {
            //cout << "vertexPtr is null" << endl;
            continue;
        }
        if(vertexPtr != NULL) {
            vertex = vertexPtr->GetPos();
            //cout << "vertex: (" << vertex.X() << ", " << vertex.Y() << ", " << vertex.Z() << ")" << endl;

            vtxHistZ->Fill(vertex.Z());

            if(vertex.Z() < fVertZPosCut - fVertZSigCut * 3 || vertex.Z() > fVertZPosCut + fVertZSigCut * 3)
               continue;

            vtxXY->Fill(vertex.X(), vertex.Y() + 240);
            vtxX->Fill(vertex.X());
            vtxY->Fill(vertex.Y() +240);
            if(useBDC) {
               vtxVtbdcX->Fill(vertex.X(), tbdc_x);
               vtxVtbdcY->Fill(vertex.Y() + 240, tbdc_y);
               tbdcXY->Fill(tbdc_x, tbdc_y);
               tbdcX->Fill(tbdc_x);
               tbdcY->Fill(tbdc_y);
            }
        }
    }

    TF1 *fit =  new TF1("fit", "gaus", -50, 50);

    TCanvas *c1 = new TCanvas("c1", "c1", 1);
    vtxXY->Draw("COLZ");

    TCanvas *c2 = new TCanvas("c2", "c2", 1);
    tbdcXY->Draw("COLZ");

    TCanvas *c3 = new TCanvas("c3", "c3", 1);
    vtxVtbdcX->Draw("COLZ");

    TCanvas *c4 = new TCanvas("c4", "c4", 1);
    vtxVtbdcY->Draw("COLZ");

    TCanvas *c5 = new TCanvas("c5", "c5", 1);
    vtxHistZ->Draw();

    TCanvas *c6 = new TCanvas("c6", "c6", 1);
    vtxX->Draw("COLZ");
    vtxX->Fit("fit");
    double avgVtxX = fit->GetParameter(1);

    TCanvas *c7 = new TCanvas("c7", "c7", 1);
    tbdcX->Draw("COLZ");
    tbdcX->Fit("fit");
    double avgTbdcX = fit->GetParameter(1);

    TCanvas *c8 = new TCanvas("c8", "c8", 1);
    vtxY->Draw("COLZ");
    vtxY->Fit("fit");
    double avgVtxY = fit->GetParameter(1);

    TCanvas *c9 = new TCanvas("c9", "c9", 1);
    tbdcY->Draw("COLZ");
    tbdcY->Fit("fit");
    double avgTbdcY = fit->GetParameter(1);

    double shiftX = avgVtxX - avgTbdcX;
    double shiftY = avgVtxY - avgTbdcY;

    cout << "Shift X: " << shiftX << "; Shift Y: " << shiftY - 240 << endl;

}