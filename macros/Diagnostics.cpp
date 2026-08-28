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

void Diagnostics(int runNum = 3154) {

    TString beamDataPathWithFormat = "/mnt/spirit/rawdata/misc/Frozen_Information_For_SpiRIT_Analysis/Aug2019/BeamData/beam/beam_run%d.ridf.root";
    TString fBeamData = Form(beamDataPathWithFormat.Data(), runNum);

    Double_t fZ, fAoQ, fBeta37;
    Double_t fBDC1x, fBDC1y, fBDC2x, fBDC2y, fBDCax, fBDCby;

    auto fBeamFile = new TFile(fBeamData);
    auto fBeamTree = (TTree *) fBeamFile -> Get("TBeam");
    fBeamTree -> SetBranchAddress("z", &fZ);
    fBeamTree -> SetBranchAddress("aoq", &fAoQ);
    fBeamTree -> SetBranchAddress("beta37", &fBeta37);
    auto fBDCTree = (TTree *) fBeamFile -> Get("TBDC");
    fBDCTree -> SetBranchAddress("bdc1x", &fBDC1x);
    fBDCTree -> SetBranchAddress("bdc1y", &fBDC1y);
    fBDCTree -> SetBranchAddress("bdc2x", &fBDC2x);
    fBDCTree -> SetBranchAddress("bdc2y", &fBDC2y);
    fBDCTree -> SetBranchAddress("bdcax", &fBDCax);
    fBDCTree -> SetBranchAddress("bdcby", &fBDCby);

    auto fBeamEnergy = new STBeamEnergy();
    fBeamEnergy -> setBeam(runNum);
    auto fBDCProjection = new STBDCProjection(TString(gSystem -> Getenv("VMCWORKDIR")) + "/parameters/ReducedBMap.txt");
    fBDCProjection -> setBeam(runNum);

    auto beamCount = fBeamTree -> GetEntries();

    TString filePath = TString::Format("data/run%04d_s0.reco.test.root", runNum);
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    TH1D *vtxHistX = new TH1D("vtxHistX", "vtxHistX", 100, -100, 100);
    TH1D *vtxHistY = new TH1D("vtxHistY", "vtxHistY", 100, -250, -200);
    TH1D *vtxHistZ = new TH1D("vtxHistZ", "vtxHistZ", 100, -100, 100);

    TH1D *bdcHistX = new TH1D("bdcHistX", "bdcHistX", 100, -100, 100);
    TH1D *bdcHistY = new TH1D("bdcHistY", "bdcHistY", 100, -25, 25);
    TH1D *bdcHistZ = new TH1D("bdcHistZ", "bdcHistZ", 100, -680, -480);

    TH2D *vtxVbdcX = new TH2D("vtxVbdcX", "vtxVbdcX", 100, -25, 25, 100, -25, 25);
    TH2D *vtxVbdcY = new TH2D("vtxVbdcY", "vtxVbdcY", 100, -250, -200, 100, -25, 25);

    TH2D *pidHist = new TH2D("pidHist", "pidHist", 1000, -1000, 2000, 1000, 0, 1000);
    TH1I *multiplicity = new TH1I("multiplicity", "multiplicity", 100, 0, 100);
    TH2D *katanaPos = new TH2D("katanaPos", "katanaPos", 130, -260, 260, 50, -15, 15);

    TH2D *thetPhi = new TH2D("thetPhi", "thetPhi", 100, 0, 90, 100, 0, 360);

    int eventCount = tree->GetEntries();

    cout << "eventCount: " << eventCount << "; beamCount: " << beamCount << endl;

    for(int i = 0; i < eventCount; i++) {

        fBeamTree -> GetEntry(i);
        fBDCTree -> GetEntry(i);
        fBeamEnergy -> reset(fZ, fAoQ, fBeta37);

        Double_t E1 = fBeamEnergy -> getCorrectedEnergy();

        if (fZ > 0 && fZ < 75 && fAoQ > 1. && fAoQ < 3 && fBDC1x > -999 && fBDC1y > -999 && fBDC2x > -999 && fBDC2y > -999) {
//          Double_t ProjectedAtZ = -580.4 + vertex -> GetPos().Z();  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
            //Double_t ProjectedAtZ = -580.4 + (fPeakZ != -9999 ? fPeakZ : vertex -> GetPos().Z());  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
            Double_t ProjectedAtZ = -580.4;  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
//          double ProjectedAtZ=-592.644;//////mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
            fBDCProjection -> ProjectParticle(fBDC2x, fBDC2y, -2160., fBDCax, fBDCby, fZ, E1, ProjectedAtZ, fBeamEnergy -> getMass());//-580.4,-583.904
        }

        //cout << "bdc: (" << fBDCProjection -> getX() << ", " << fBDCProjection -> getY() << ", " << fBDCProjection -> getZ() << ")" << endl;
        
        bdcHistX->Fill(fBDCProjection -> getX());
        bdcHistY->Fill(fBDCProjection -> getY());
        bdcHistZ->Fill(fBDCProjection -> getZ());

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
            //cout << "vertex: (" << vertex.X() << ", " << vertex.Y() << ", " << vertex.Z() << ")" << endl;
            vtxHistX->Fill(vertex.X());
            vtxHistY->Fill(vertex.Y());
            vtxHistZ->Fill(vertex.Z());

            vtxVbdcX->Fill(vertex.X(), fBDCProjection -> getX());
            vtxVbdcY->Fill(vertex.Y(), fBDCProjection -> getY());
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

    //for(int i = 0; i < beamCount; i++) {

    //}

    TCanvas *cVX = new TCanvas("cVX", "cVX", 1);
    vtxHistX->Draw();

    TCanvas *cVY = new TCanvas("cVY", "cVY", 1);
    vtxHistY->Draw();

    TCanvas *cVZ = new TCanvas("cVZ", "cVZ", 1);
    vtxHistZ->Draw();

    TCanvas *cBX = new TCanvas("cBX", "cBX", 1);
    bdcHistX->Draw();

    TCanvas *cBY = new TCanvas("cBY", "cBY", 1);
    bdcHistY->Draw();

    TCanvas *cBZ = new TCanvas("cBZ", "cBZ", 1);
    bdcHistZ->Draw();
    
    TCanvas *cVBX = new TCanvas("cVBX", "cVBX", 1);
    vtxVbdcX->Draw("COLZ");

    TCanvas *cVBY = new TCanvas("cVBY", "cVBY", 1);
    vtxVbdcY->Draw("COLZ");

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
