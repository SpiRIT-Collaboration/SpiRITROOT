namespace piSelect {
    
FairRootManager *fRootManager;
FairRunAna *fRunAna;
STAuxHeader *fInputAuxHeader;
TClonesArray *fRecoTrackArray = nullptr;
TClonesArray *fRecoVATrackArray = nullptr;
TClonesArray *fVertexArray = nullptr;
STBeamInfo *fBeamInfo = nullptr;

TString fInputBranchName{"STAuxHeaderLinked"}; // Name if AtRawEvent branch
STAuxHeader *fOutputAuxHeader;       // AtRawEvent
TString fOutputBranchName{"STAuxHeaderM"}; // Name if AtRawEvent branch

TClonesArray *fRecoVAOutArray = nullptr;
TClonesArray *fVertexOutArray = nullptr;
TParameter<Int_t> *fMulti = nullptr;

TF1 *fit;

//Vertex Z parameters
double targetZpos = -21.86;
//double targetZpos = -32.8;
//double targetZsig = 2.15;
double targetZsig = 2.5;

//Vertex X parameters
double targetXwidth = 30;

//Window X parameter
double windowXwidth = 57.3;

int fClustCut = 15;
double fPOCACut = 20;

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

void Init() {
    fRootManager = FairRootManager::Instance();
    if (fRootManager == nullptr) {
        cout << "Cannot find RootManager!" << endl;
    }

    fRunAna = FairRunAna::Instance();
    if (fRunAna == nullptr) {
        cout << "Cannot find RunAna!" << endl;
    }

    fInputAuxHeader = (STAuxHeader *)(fRootManager -> GetObject(fInputBranchName));
    if (fInputAuxHeader == nullptr) {
        cout << TString::Format("Cannot find STAuxHeader array in branch %s !", fInputBranchName.Data()).Data() << endl;
    }

    fRecoTrackArray = (TClonesArray *) fRootManager -> GetObject("STRecoTrack");
    if (fRecoTrackArray == nullptr) {
        cout << "Cannot find STRecoTrack array!" << endl;
    }

    fRecoVATrackArray = (TClonesArray *) fRootManager -> GetObject("VATracks");
    if (fRecoVATrackArray == nullptr) {
        cout << "Cannot find VATracks array!" << endl;
    }

    fVertexArray = (TClonesArray *) fRootManager -> GetObject("STVertex");
    if (fVertexArray == nullptr) {
        cout << "Cannot find STVertex array!" << endl;
    }

    fBeamInfo = (STBeamInfo *)(fRootManager -> GetObject("STBeamInfo"));
    if (fInputAuxHeader == nullptr) {
        cout << "Cannot find STBeamInfo!" << endl;
    }

    // Create the new Aux Header
    fOutputAuxHeader = new STAuxHeader();
    fRootManager->Register(fOutputBranchName, "SpiRIT", fOutputAuxHeader, true);

    fRootManager->Register("PiVertex", "SpiRIT", fVertexArray, true);

    fRecoVAOutArray = new TClonesArray("STRecoTrack2024");
    fRootManager->Register("PiTracks", "SpiRIT", fRecoVAOutArray, true);

    fMulti = new TParameter<Int_t>("muliti", 0);
    fRootManager->RegisterAny("Multiplicity", fMulti, true);

    fRootManager->Register("PiBeamInfo", "SpiRIT", fBeamInfo, true);

    fit = new TF1("fit", fpFunc, -500, 2500, 5);
    fit->SetParameters(6.48253e-1, 2.85595e1, 2.30199e-1, 2.05635, -2.35960);

}

void Exec() {
    fOutputAuxHeader -> Clear();
    fRecoVAOutArray->Clear();

    fOutputAuxHeader -> SetTpcEventNum(fInputAuxHeader -> GetTpcEventNum());
    fOutputAuxHeader -> SetTpcTime(fInputAuxHeader -> GetTpcTime());
    fOutputAuxHeader -> SetBdcID(fInputAuxHeader -> GetBdcID());

    auto trackCount = fRecoTrackArray->GetEntries();
    auto trackVACount = fRecoVATrackArray->GetEntries();

    auto vertexPtr = dynamic_cast<STVertex *>(fVertexArray->At(0));

    if(vertexPtr == NULL) {
        //cout << "vertexPtr is null" << endl;
        fRunAna->MarkFill(false);
        return;
    }
    auto vertex = vertexPtr->GetPos();
    //cout << "vertex: (" << vertex.X() << ", " << vertex.Y() << ", " << vertex.Z() << ")" << endl;
    if(vertex.Z() > targetZpos + 3 * targetZsig || vertex.Z() < targetZpos - 3 * targetZsig) {
        //cout << "Not on target Z. Not Filling." << endl;
        fRunAna->MarkFill(false);
        return;
    }
    if(vertex.X() > targetXwidth / 2 || vertex.X() < -targetXwidth / 2) {
        //cout << "Not on target X. Not Filling." << endl;
        fRunAna->MarkFill(false);
        return;
    }

    int multi = 0;

    for(int r = 0; r < trackCount; r++) {
        auto recoPtr = dynamic_cast<STRecoTrack2024 *>(fRecoTrackArray->At(r));

        if(recoPtr == NULL) {
            cout << "recoPtr is null" << endl;
            continue;
        }
        if(recoPtr->GetNumLayerClusters() + recoPtr->GetNumRowClusters() > fClustCut && (recoPtr->GetPOCAVertex() - vertex).Mag() < fPOCACut) {
            multi++;
        }
    }

    fMulti->SetVal(multi);

    for(int r = 0; r < trackCount; r++) {
        auto recoPtr = dynamic_cast<STRecoTrack2024 *>(fRecoTrackArray->At(r));
        STRecoTrack2024 *recoVAPtr;

        if(recoPtr == NULL) {
            cout << "recoPtr is null" << endl;
            continue;
        }
        if(recoPtr->GetRecoID() < 0)
            continue;
        for(int a = 0; a < trackVACount; a++) {
            recoVAPtr = dynamic_cast<STRecoTrack2024 *>(fRecoVATrackArray->At(a));
            if(recoPtr->GetRecoID() == recoVAPtr->GetRecoID())
                break;
        }
        auto trackL = recoPtr->GetTrackLength();
        auto clusters = recoPtr->GetNumLayerClusters() + recoVAPtr->GetNumRowClusters();
        
        //if(recoPtr->GetNumLayerClusters() + recoPtr->GetNumRowClusters() > fClustCut && (recoPtr->GetPOCAVertex() - vertex).Mag() < fPOCACut && recoPtr->GetPosWindow().X() < windowXwidth / 2 && recoPtr->GetPosWindow().X() > -windowXwidth / 2) {
        if(recoPtr->GetNumLayerClusters() + recoPtr->GetNumRowClusters() > fClustCut && (recoPtr->GetPOCAVertex() - vertex).Mag() < fPOCACut) {
            auto mom = recoVAPtr->GetMomentum().Mag();
            auto dedx = recoVAPtr->GetdEdxWithCut(0, 0.7, 0.5);
            auto charge = recoVAPtr->GetCharge();
            auto gfCharge = recoVAPtr->GetGenfitCharge();

            auto momTgt = recoVAPtr->GetMomentumTargetPlane(); 
            auto theta = momTgt.Theta() * TMath::RadToDeg();
            auto phiDeg = momTgt.Phi() * TMath::RadToDeg();
            auto ang = atan(abs(momTgt.X()) / momTgt.Z()) * TMath::RadToDeg();

            //if(!(phiDeg > -40 && phiDeg < 25) && !(phiDeg > 160 && phiDeg < 180) && !(phiDeg > -180 && phiDeg < -150))
            //     continue;

            if(mom * gfCharge > -700 && mom * gfCharge < 700 && log10(dedx / fit->Eval(mom)) < 0.5) {
                auto trackID = fRecoVAOutArray->GetEntriesFast();
                new ((*fRecoVAOutArray)[trackID]) STRecoTrack2024(recoVAPtr);
            }
        }
    }

}

}