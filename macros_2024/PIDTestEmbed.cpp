TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;
TTreeReaderValue<TClonesArray> *embedReader = nullptr;
TTreeReaderValue<TClonesArray> *recoVAReader = nullptr;
TTreeReaderValue<TParameter<Int_t>> *multiReader = nullptr;

STVertex *vertexPtr;
STRecoTrack2024 *recoPtr;
STRecoTrack2024 *recoVAPtr;
STEmbedTrack *embedPtr;

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

void PIDTestEmbed(int runNum = 1191, int emSpec = 211, int fMultiCut = 15, int fClustCut = 15, double fPOCACut = 20) {
    string spec = "136Xe";
    if(runNum < 1600) {
        targetZpos = -21.86;
        spec = "124Xe";
    }
    //TString filePath = TString::Format("./BulkReconstruction/run%04d*.root", runNum);
    //auto filePath = TString::Format("./Embed251020/run%04d*_%d*.root", runNum, emSpec);
    auto filePath = TString::Format("./251024_embed/run*_%d*.root", emSpec);
    cout << filePath << endl;
    //TString filePath = TString::Format("../../curtis/MarSCdensity/BulkTest/dXXXX/run%04d*.root", runNum);
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    //VAVertex and VATracks use the vertex from the BDC
    //vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "VAVertex");
    //recoReader = new TTreeReaderValue<TClonesArray>(*reader, "PiTracks");
    embedReader = new TTreeReaderValue<TClonesArray>(*reader, "STEmbedTrack");

    //STVertex and STRecoTrack use the vertex from the tracks
    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    //recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    //multiReader = new TTreeReaderValue<TParameter<Int_t>>(*reader, "Multiplicity");

    TH2D *pidVAHist = new TH2D("pidVAHist", "pidVAHist", 1000, -500, 2500, 1000, 0, 2000);
    TH2D *pidFlatLog = new TH2D("pidFlatLog", "pidFlatLog", 1000, -700, 700, 1000, -3, 3);

    TH2D *phiTheta = new TH2D("phiTheta", "phiTheta", 45, 0, 90, 180, -180, 180);
    TH2D *phiThetaEm = new TH2D("phiThetaEm", "phiThetaEm", 90, 0, 90, 360, -180, 180);
    TH2D *phiThetaNet = new TH2D("phiThetaNet", "phiThetaNet", 45, 0, 90, 180, -180, 180);
    TH1D *momHist = new TH1D("momHist", "momHist", 700, 0, 700);

    TH2D *momComp =  new TH2D("momComp", "momComp", 700, 0, 700, 700, 0, 700);
    TH2D *thetaComp = new TH2D("thetaComp", "thetaComp", 90, 0, 90, 90, 0, 90);
    TH2D *phiComp = new TH2D("phiComp", "phiComp", 360, -180, 180, 360, -180, 180);

    TH2D *momTheta0 = new TH2D("momTheta0", "momTheta0", 34, 50, 560, 16, 0, 72);
    TH2D *momTheta1 = new TH2D("momTheta1", "momTheta1", 34, 50, 560, 16, 0, 72);
    TH2D *momTheta = new TH2D("momTheta", "momTheta", 34, 50, 560, 16, 0, 72);
    TH2D *momTheta0Net = new TH2D("momTheta0Net", "momTheta0Net", 34, 50, 560, 16, 0, 72);
    TH2D *momTheta1Net = new TH2D("momTheta1Net", "momTheta1Net", 34, 50, 560, 16, 0, 72);
    TH2D *momThetaNet = new TH2D("momThetaNet", "momThetaNet", 34, 50, 560, 16, 0, 72);

    TH1D *phiRaw0 = new TH1D("phiRaw0", "phiRaw0", 180, -180, 180);
    TH1D *phiRaw1 = new TH1D("phiRaw1", "phiRaw1", 180, -180, 180);

    int eventCount = tree->GetEntries();

    TF1 *fit = new TF1("fit", fpFunc, -500, 2500, 5);
    fit->SetParameters(6.48253e-1, 2.85595e1, 2.30199e-1, 2.05635, -2.35960);

    int piCount = 0;
    int netCount = 0;
    int phiCount = 0;

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        auto trackCount = embedReader->Get()->GetEntries();
        vertexPtr = dynamic_cast<STVertex *>((*vertexReader)->At(0));
        if(vertexPtr == NULL) {
            cout << "vertexPtr is null" << endl;
            continue;
        }
        auto vertex = vertexPtr->GetPos();
        
        for(int r = 0; r < trackCount; r++) {
            embedPtr = dynamic_cast<STEmbedTrack *>((*embedReader)->At(r));

            if(embedPtr == NULL) {
                cout << "embedPtr is null" << endl;
                continue;
            }

            auto recoTrack = (STRecoTrack2024 *)embedPtr->GetFinalTrack(); 


            auto mom = recoTrack->GetMomentum().Mag();
            auto dedx = recoTrack->GetdEdxWithCut(0, 0.7, 0.5);
            auto charge = recoTrack->GetCharge();
            auto gfCharge = recoTrack->GetGenfitCharge();

            auto dedxAdj = log10(dedx / fit->Eval(mom));

            auto momTgt = recoTrack->GetMomentumTargetPlane(); 
            auto theta = momTgt.Theta() * TMath::RadToDeg();
            auto phiDeg = momTgt.Phi() * TMath::RadToDeg();
            auto ang = atan(abs(momTgt.X()) / momTgt.Z()) * TMath::RadToDeg();

            auto momEm = embedPtr->GetInitialMom().Mag() * 1000;
            auto momTgtEm = embedPtr->GetInitialMom();

            //cout << "Init Moment: " << momEm << "; Moment_X: " << momTgtEm.X() << "; Moment_Y: " << momTgtEm.Y() << "; Moment_Z: " << momTgtEm.Z() << endl;
            //cout << "Init Theta: " << momTgtEm.Theta() * TMath::RadToDeg() << "; Init Phi: " << momTgtEm.Phi() * TMath::RadToDeg() << endl;

            auto thetaEm = momTgtEm.Theta() * TMath::RadToDeg();
            auto phiDegEm = momTgtEm.Phi() * TMath::RadToDeg();


            netCount++;
            if(!((phiDegEm > -40 && phiDegEm < 25) || (phiDegEm > 160 && phiDegEm < 180) || (phiDegEm > -180 && phiDegEm < -150))) {
                continue;
            }
                phiCount++;

            phiThetaNet->Fill(thetaEm, phiDegEm);

            if(recoTrack->GetNumLayerClusters() + recoTrack->GetNumRowClusters() > fClustCut && (recoTrack->GetPOCAVertex() - vertex).Mag() < fPOCACut && recoTrack->GetPosWindow().X() < windowXwidth / 2 && recoTrack->GetPosWindow().X() > -windowXwidth / 2) {
                if(!(dedx < 0)) {
                    pidVAHist->Fill(mom * gfCharge, dedx);
                    pidFlatLog->Fill(mom * gfCharge, dedxAdj);
                    if(dedxAdj < 0.15 && dedxAdj > -0.15) {
                        phiTheta->Fill(theta, phiDeg);
                        momComp->Fill(momEm, mom);
                        thetaComp->Fill(thetaEm, theta);
                        phiComp->Fill(phiDegEm, phiDeg);
                        if((phiDeg > 160 && phiDeg < 180) || (phiDeg > -180 && phiDeg < -150)) {
                            momTheta0->Fill(momEm, thetaEm);
                            momTheta->Fill(momEm, thetaEm);
                            phiRaw0->Fill(phiDegEm);
                            piCount++;
                        }
                        else if(phiDeg > -40 && phiDeg < 25) {
                            momTheta1->Fill(momEm, thetaEm);
                            momTheta->Fill(momEm, thetaEm);
                            phiRaw1->Fill(phiDegEm);
                            piCount++;
                        }
                  }
                }
                else {
                    phiThetaEm->Fill(thetaEm, phiDegEm);
                    momHist->Fill(momEm);
                }
            }

            if((phiDegEm > 160 && phiDegEm < 180) || (phiDegEm > -180 && phiDegEm < -150)){
                momTheta0Net->Fill(momEm, thetaEm);
                momThetaNet->Fill(momEm, thetaEm);
            }
            else if(phiDegEm > -40 && phiDegEm < 25) {
                momTheta1Net->Fill(momEm, thetaEm);
                momThetaNet->Fill(momEm, thetaEm);
            }

            /*if((phiDeg > 160 && phiDeg < 180) || (phiDeg > -180 && phiDeg < -150)) {
                phiCount++;
            }
            else if(phiDeg > -40 && phiDeg < 25) {
                phiCount++;
            }*/

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

            auto value0Net = momTheta0Net->GetBinContent(binX, binY);
            momTheta0Net->SetBinError(binX, binY, sqrt(value0Net));
            auto value1Net = momTheta1Net->GetBinContent(binX, binY);
            momTheta1Net->SetBinError(binX, binY, sqrt(value1Net));
            auto valueNet = momThetaNet->GetBinContent(binX, binY);
            momThetaNet->SetBinError(binX, binY, sqrt(valueNet));
        }
    }

    TH2D *momTheta0Rat = new TH2D(*momTheta0);
    momTheta0Rat->Divide(momTheta0Net);
    momTheta0Rat->SetNameTitle("momTheta0Rat", "momTheta0Rat");

    TH2D *momTheta1Rat = new TH2D(*momTheta1);
    momTheta1Rat->Divide(momTheta1Net);
    momTheta1Rat->SetNameTitle("momTheta1Rat", "momTheta1Rat");

    TH2D *momThetaRat = new TH2D(*momTheta);
    momThetaRat->Divide(momThetaNet);
    momThetaRat->SetNameTitle("momThetaRat", "momThetaRat");

    cout << "momThetaRat Error (10, 10): " << momThetaRat->GetBinError(10, 10) << "; momTheta Error (10, 10): " << momTheta->GetBinError(10, 10) << "; momThetaNet Error (10, 10): " << momThetaNet->GetBinError(10, 10) << endl;
    cout << "momThetaRat expected Error (10, 10): " << momThetaRat->GetBinContent(10, 10) * sqrt(pow(momTheta->GetBinError(10, 10) / momTheta->GetBinContent(10, 10), 2) + pow(momThetaNet->GetBinError(10, 10) / momThetaNet->GetBinContent(10, 10), 2)) << endl;

    //TString outFilePath = TString::Format("PIDtestMayMeet/pid_%04d_d0000.root", runNum);
    //if(isSC)
    //    outFilePath = TString::Format("PIDtestMayMeet/pid_%04d_dXXXX.root", runNum);

    cout << "Total Events: " << netCount << "; Total Pions: " << piCount << endl;
    cout << "Total Phi Events: " << phiCount << "; Total Pions: " << piCount << endl;

    //TString outFilePath = TString::Format("./data/pidEmbed251022/pid_%04d.root", runNum);
    TString outFilePath = TString::Format("./data/pidEmbed251022/pid_%d_251024.root", emSpec);

    TFile *outFile = new TFile(outFilePath.Data(), "RECREATE");
    pidVAHist->Write();
    pidFlatLog->Write();
    phiTheta->Write();
    phiThetaEm->Write();
    phiThetaNet->Write();
    momHist->Write();
    momComp->Write();
    thetaComp->Write();
    phiComp->Write();
    momTheta0->Write();
    momTheta1->Write();
    momTheta->Write();
    momTheta0Net->Write();
    momTheta1Net->Write();
    momThetaNet->Write();
    momTheta0Rat->Write();
    momTheta1Rat->Write();
    momThetaRat->Write();
    phiRaw0->Write();
    phiRaw1->Write();

    gApplication->Terminate();

}
