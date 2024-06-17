TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;

STRecoTrack *recoPtr;

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

const double pi = 3.1415926;

void BeamDiag(int runNum = 3154) {

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

    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    TH1D *tpcHistX = new TH1D("tpcHistX", "tpcHistX", 100, -25, 25);
    TH1D *tpcHistY = new TH1D("tpcHistY", "tpcHistY", 100, -25, 25);

    TH1D *bdcHistX = new TH1D("bdcHistX", "bdcHistX", 100, -25, 25);
    TH1D *bdcHistY = new TH1D("bdcHistY", "bdcHistY", 100, -25, 25);

    TH2D *tpcVbdcX = new TH2D("tpcVbdcX", "tpcVbdcX", 100, -25, 25, 100, -25, 25);
    TH2D *tpcVbdcY = new TH2D("tpvVbdcY", "tpcVbdcY", 100, -25, 25, 100, -25, 25);

    TH2D *katanaPos = new TH2D("katanaPos", "katanaPos", 130, -260, 260, 50, -15, 15);
    TH1D *katanaX = new TH1D("katanaX", "katanaX", 100, 0, 260);

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

        reader->Next();
        auto trackCount = recoReader->Get()->GetEntries();
        cout << "event: " << i << endl;
        cout << "recoEntries: " << trackCount << endl;

        for(int r = 0; r < trackCount; r++) {
            recoPtr = dynamic_cast<STRecoTrack *>((*recoReader)->At(r));

            if(recoPtr == NULL) {
                cout << "recoPtr is null" << endl;
            }
            if(recoPtr != NULL) {

                auto posTarg = recoPtr->GetPosTargetPlane();
                if(posTarg.X() == 0)
                    break;
                tpcHistX->Fill(posTarg.X());
                tpcHistY->Fill(posTarg.Y() + 225);

                tpcVbdcX->Fill(posTarg.X(), fBDCProjection -> getX());
                tpcVbdcY->Fill(posTarg.Y() + 225, fBDCProjection -> getY());

                auto posKat = recoPtr->GetPosKatana();
                katanaPos->Fill(posKat.X(), posKat.Y());
                katanaX->Fill(posKat.X());
                //cout << "track: " << r << "; mom: " << mom << "; dEdx: " << dedx << endl;

            }
        }
    }

    //for(int i = 0; i < beamCount; i++) {

    //}

    TCanvas *cTX = new TCanvas("cTX", "cTX", 1);
    tpcHistX->Draw();

    TCanvas *cTY = new TCanvas("cTY", "cTY", 1);
    tpcHistY->Draw();

    TCanvas *cBX = new TCanvas("cBX", "cBX", 1);
    bdcHistX->Draw();

    TCanvas *cBY = new TCanvas("cBY", "cBY", 1);
    bdcHistY->Draw();

    TCanvas *cTBX = new TCanvas("cTBX", "cTBX", 1);
    tpcVbdcX->Draw("COLZ");

    TCanvas *cTBY = new TCanvas("cTBY", "cTBY", 1);
    tpcVbdcY->Draw("COLZ");

    TCanvas *cKat = new TCanvas("cKat", "cKat", 1);
    katanaX->Draw();

    //TFile *outFile = new TFile(outFileName, "RECREATE");
    //pidHist->Write();
    //outFile->Close();



}
