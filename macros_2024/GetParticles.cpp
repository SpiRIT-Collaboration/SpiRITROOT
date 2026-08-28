TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<STAuxHeader> *auxHeadReader = nullptr;
TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
TTreeReaderValue<TClonesArray> *recoReader = nullptr;
TTreeReaderValue<TClonesArray> *recoVAReader = nullptr;
TTreeReaderValue<TParameter<Int_t>> *multiReader = nullptr;
TTreeReaderValue<STBeamInfo> *beamInfoReader = nullptr;

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

//BDC Offsets
double bdcOffsetX = -2.70;
double bdcOffsetY = -181.8;

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

void GetParticles(int runNum = 1191, int fMultiCut = 46, int fClustCut = 15, double fPOCACut = 20) {
    string spec = "136Xe";
    if(runNum < 1600) {
        //targetZpos = -21.86;
        targetZpos = -19.48;
        spec = "124Xe";
        fMultiCut = 45;
    }
    TString filePath = TString::Format("./BulkReconstruction/run%04d*.root", runNum);
    //auto filePath = TString::Format("./data_reduc/%s/run*.root", spec.c_str());
    //auto filePath = TString::Format("./data_reduc/%s/run%04d_s*.root", spec.c_str(), runNum);
    cout << filePath << endl;
    //TString filePath = TString::Format("../../curtis/MarSCdensity/BulkTest/dXXXX/run%04d*.root", runNum);
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    auxHeadReader = new TTreeReaderValue<STAuxHeader>(*reader, "STAuxHeaderM");

    //VAVertex and VATracks use the vertex from the BDC
    //vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "VAVertex");
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "PiTracks");

    //STVertex and STRecoTrack use the vertex from the tracks
    //vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "PiVertex");
    //recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    multiReader = new TTreeReaderValue<TParameter<Int_t>>(*reader, "Multiplicity");
    beamInfoReader = new TTreeReaderValue<STBeamInfo>(*reader, "PiBeamInfo");

    int eventCount = tree->GetEntries();

    TF1 *fit = new TF1("fit", fpFunc, -500, 2500, 5);
    fit->SetParameters(6.48253e-1, 2.85595e1, 2.30199e-1, 2.05635, -2.35960);

    ofstream outFileVert;
    outFileVert.open(TString::Format("./data_sim_parts/beamVertex%04d_19_48.txt", runNum).Data());
    outFileVert << "#RunNum   EventNum    x(cm)    y(cm)    z(cm)" << endl;

    ofstream outFilePart;
    outFilePart.open(TString::Format("./data_sim_parts/particle%04d_19_48.txt", runNum).Data());
    outFilePart << "#RunNum   EventNum    p(MeV/c)    theta(deg)    phi(deg)" << endl;

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        vertexPtr = dynamic_cast<STVertex *>((*vertexReader)->At(0));
        auto auxHeader = (STAuxHeader *)auxHeadReader->Get();

        if(vertexPtr == NULL) {
            //cout << "vertexPtr is null" << endl;
            continue;
        }
        auto vertex = vertexPtr->GetPos();
        auto beamInfo = (STBeamInfo *)beamInfoReader->Get(); 
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

            if(!(phiDeg > -40 && phiDeg < 25) && !(phiDeg > 160 && phiDeg < 180) && !(phiDeg > -180 && phiDeg < -150))
                 continue;

            //if(gfCharge > 0)
            //    continue;

            if(dedxAdj < 0.20 && dedxAdj > -0.20 && gfCharge > 0) {
                outFileVert << runNum << "  " << auxHeader->GetTpcEventNum() << "  " << (beamInfo->fXTargetPlane + bdcOffsetX) / 10. << "  " << (beamInfo->fYTargetPlane + bdcOffsetY) / 10. << "  " << targetZpos / 10. << endl;
                outFilePart << runNum << "  " << auxHeader->GetTpcEventNum() << "  " << mom << "  " << theta << "  " << phiDeg << endl;

            }
            
            //cout << "track: " << r << "; mom: " << mom << "; dEdx: " << dedx << endl;
        }
    }

    outFileVert.close();
    outFilePart.close();


    gApplication->Terminate();

}
