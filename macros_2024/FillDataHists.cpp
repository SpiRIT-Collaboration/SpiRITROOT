TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;
TTreeReaderValue<TClonesArray> *recoVAReader = nullptr;
TTreeReaderValue<TParameter<Int_t>> *multiReader = nullptr;

STVertex *vertexPtr;
STRecoTrack2024 *recoPtr;
STRecoTrack2024 *recoVAPtr;



//Vertex Z parameters
//double targetZpos = -21.86;
double targetZpos = -32.8;
//double targetZsig = 2.15;
double targetZsig = 2.5;

//Vertex X parameters
double targetXwidth = 30;

//Window X parameter
double windowXwidth = 57.3;

//Cut on number of clusters for a given track
//Int_t fClustCut = 15;

//Cut to check track actually originates from the vertex
//Double_t fPOCACut = 20;

//Cut on multiplicity
//Int_t fMultiCut = 15;

auto mpi = 139.57018;
auto me = 0.5109989461;
auto mp = 938.2720813;
auto mn = 939.565346;
auto md = 1875.612762;
auto mt = 2808.921112;
auto mhe3 = 2808.39132;
auto mal = 3727.379378;

const Double_t particleMass = mpi; // Select particle you'll going to analyze

Double_t fpFunc(Double_t *x, Double_t *p) {
  if (x[0] < 0)
    x[0] = -x[0];

  const Double_t m = particleMass;
  Double_t totalE = TMath::Sqrt(m*m + x[0]*x[0]);
  Double_t beta = x[0]/totalE;
  Double_t gamma = 1./TMath::Sqrt(1 - beta*beta);

  Double_t betaP3 = TMath::Power(beta, p[3]);

  return p[0]/betaP3*(p[1] - betaP3 + TMath::Log(p[2] + TMath::Power(1./(beta*gamma), p[4])));
}

void FillDataHists(int runNum = 1191, int fMultiCut = 46, int fClustCut = 15, double fPOCACut = 20) {
    string spec = "136Xe";
    if(runNum < 1600) {
        targetZpos = -21.86;
        spec = "124Xe";
        fMultiCut = 45;
    }
    //TString filePath = TString::Format("./BulkReconstruction/run%04d*.root", runNum);
    //auto filePath = TString::Format("./data_reduc/%s/run*.root", spec.c_str());
    auto filePath = TString::Format("./data_reduc_noWin/%s/run*.root", spec.c_str());
    cout << filePath << endl;
    //TString filePath = TString::Format("../../curtis/MarSCdensity/BulkTest/dXXXX/run%04d*.root", runNum);
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    //VAVertex and VATracks use the vertex from the BDC
    //vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "VAVertex");
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "PiTracks");

    //STVertex and STRecoTrack use the vertex from the tracks
    //vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    //recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    multiReader = new TTreeReaderValue<TParameter<Int_t>>(*reader, "Multiplicity");

    TH2D *pidFlatLog = new TH2D("pidFlatLog", "pidFlatLog", 700, -700, 700, 1000, -3, 0.5);

    TH2D *phiTheta = new TH2D("phiTheta", "phiTheta", 45, 0, 90, 180, -180, 180);
    TH2D *momTheta0 = new TH2D("momTheta0", "momTheta0", 34, 50, 560, 16, 0, 72);
    TH2D *momTheta1 = new TH2D("momTheta1", "momTheta1", 34, 50, 560, 16, 0, 72);
    TH2D *momTheta = new TH2D("momTheta", "momTheta", 34, 50, 560, 16, 0, 72);
    TH2D *pTTheta0 = new TH2D("pTTheta0", "pTTheta0", 16, 0, 400, 16, 0, 72);
    TH2D *pTTheta1 = new TH2D("pTTheta1", "pTTheta1", 16, 0, 400, 16, 0, 72);
    TH2D *pTTheta = new TH2D("pTTheta", "pTTheta", 16, 0, 400, 16, 0, 72);

    TH1D *momPlotPlu = new TH1D("momPlotPlu", "momPlotPlu", 34, 50, 560);
    TH1D *momPlotMin = new TH1D("momPlotMin", "momPlotMin", 34, 50, 560);

    TH1D *pTPlotPlu = new TH1D("pTPlotPlu", "pTPlotPlu", 12, 0, 300);
    TH1D *pTPlotMin = new TH1D("pTPlotMin", "pTPlotMin", 12, 0, 300);

    int eventCount = tree->GetEntries();

    TF1 *fit = new TF1("fit", fpFunc, -500, 2500, 5);
    fit->SetParameters(6.48253e-1, 2.85595e1, 2.30199e-1, 2.05635, -2.35960);

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        auto trackCount = recoReader->Get()->GetEntries();
        auto multi = multiReader->Get()->GetVal();
        if(multi < fMultiCut)
            continue;
        
        for(int r = 0; r < trackCount; r++) {
            recoPtr = dynamic_cast<STRecoTrack2024 *>((*recoReader)->At(r));

            if(recoPtr == NULL) {
                cout << "recoPtr is null" << endl;
                continue;
            }
            //if(!(recoPtr->GetPosWindow().X() < windowXwidth / 2 && recoPtr->GetPosWindow().X() > -windowXwidth / 2)) {
            //if(!(recoPtr->GetNumLayerClusters() + recoPtr->GetNumRowClusters() > fClustCut)) {
            //    continue;
            //}
            auto mom = recoPtr->GetMomentum().Mag();
            auto dedx = recoPtr->GetdEdxWithCut(0, 0.7, 0.5);
            auto charge = recoPtr->GetCharge();
            auto gfCharge = recoPtr->GetGenfitCharge();

            auto dedxAdj = log10(dedx / fit->Eval(mom));

            auto momTgt = recoPtr->GetMomentumTargetPlane(); 
            auto pT = sqrt(pow(momTgt.X(), 2) + pow(momTgt.Y(), 2));
            auto theta = momTgt.Theta() * TMath::RadToDeg();
            auto phiDeg = momTgt.Phi() * TMath::RadToDeg();
            auto ang = atan(abs(momTgt.X()) / momTgt.Z()) * TMath::RadToDeg();

            pidFlatLog->Fill(mom * gfCharge, dedxAdj);

            //if(!(phiDeg > -40 && phiDeg < 25) && !(phiDeg > 160 && phiDeg < 180) && !(phiDeg > -180 && phiDeg < -150))
                 //continue;

            if(gfCharge > 0)
                continue;

            if(dedxAdj < 0.20 && dedxAdj > -0.20) {
            //if(1) {
                phiTheta->Fill(theta, phiDeg);
                if((phiDeg > 160 && phiDeg < 180) || (phiDeg > -180 && phiDeg < -150)) {
                    momTheta0->Fill(mom, theta);
                    momTheta->Fill(mom, theta);
                    pTTheta0->Fill(pT, theta);
                    pTTheta->Fill(pT, theta);
                    if(gfCharge > 0) {
                        momPlotPlu->Fill(mom);
                        pTPlotPlu->Fill(pT);
                    }
                    else {
                        momPlotMin->Fill(mom);
                        pTPlotMin->Fill(pT);
                    }
                }
                else if(phiDeg > -40 && phiDeg < 25) {
                    momTheta1->Fill(mom, theta);
                    momTheta->Fill(mom, theta);
                    pTTheta1->Fill(pT, theta);
                    pTTheta->Fill(pT, theta);
                    if(gfCharge > 0) {
                        momPlotPlu->Fill(mom);
                        pTPlotPlu->Fill(pT);
                    }
                    else {
                        momPlotMin->Fill(mom);
                        pTPlotMin->Fill(pT);
                    }
                }
            }
            
            //cout << "track: " << r << "; mom: " << mom << "; dEdx: " << dedx << endl;
        }
    }

    for(int binX = 1; binX < momTheta0->GetNbinsX() + 1; binX++) {
        for(int binY = 1; binY < momTheta0->GetNbinsY() + 1; binY++) {
            auto value0 = momTheta0->GetBinContent(binX, binY);
            momTheta0->SetBinError(binX, binY, sqrt(value0));
            auto value1 = momTheta1->GetBinContent(binX, binY);
            momTheta1->SetBinError(binX, binY, sqrt(value1));
            auto value = momTheta->GetBinContent(binX, binY);
            momTheta->SetBinError(binX, binY, sqrt(value));
        }
    }

    for(int binX = 1; binX < pTTheta0->GetNbinsX() + 1; binX++) {
        for(int binY = 1; binY < pTTheta0->GetNbinsY() + 1; binY++) {
            auto value0 = pTTheta0->GetBinContent(binX, binY);
            pTTheta0->SetBinError(binX, binY, sqrt(value0));
            auto value1 = pTTheta1->GetBinContent(binX, binY);
            pTTheta1->SetBinError(binX, binY, sqrt(value1));
            auto value = pTTheta->GetBinContent(binX, binY);
            pTTheta->SetBinError(binX, binY, sqrt(value));
        }
    }

    //TString outFilePath = TString::Format("PIDtestMayMeet/pid_%04d_d0000.root", runNum);
    //if(isSC)
    //    outFilePath = TString::Format("PIDtestMayMeet/pid_%04d_dXXXX.root", runNum);

    TString outFilePath = TString::Format("./data/pid_data_wPhi_noWin_pT/pid_%s_-211.root", spec.c_str());

    TFile *outFile = new TFile(outFilePath.Data(), "RECREATE");
    
    pidFlatLog->Write();
    phiTheta->Write();
    momTheta0->Write();
    momTheta1->Write();
    momTheta->Write();
    pTTheta0->Write();
    pTTheta1->Write();
    pTTheta->Write();
    momPlotPlu->Write();
    momPlotMin->Write();
    pTPlotPlu->Write();
    pTPlotMin->Write();

    gApplication->Terminate();

}
