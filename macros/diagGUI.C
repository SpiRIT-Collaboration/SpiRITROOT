#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <RQ_OBJECT.h>

const double pi = 3.1415926;

class DiagnosticsGUI {
    RQ_OBJECT("DiagnosticsGUI")
public:
    DiagnosticsGUI(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~DiagnosticsGUI();

    void SetRunNum() { fRunNum = fRunEnt->GetNumberEntry()->GetIntNumber(); }
    void ProcessRun();

private:
    TGMainFrame *fMain;
    TRootEmbeddedCanvas *fEcanvas[10];
    TGTab *fTab;
    TGTextEntry *fRunDisp;
    TGNumberEntry *fRunEnt;

    Int_t fRunNum = 0;

    TChain *tree = nullptr;

    TTreeReader *reader = nullptr;
    TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
    TTreeReaderValue<TClonesArray> *recoReader = nullptr;

    STVertex *vertexPtr;
    STRecoTrack *recoPtr;

    TH2D *pidHist;
    TH2D *thetPhi;
    TH1I *multiplicity;

    TH2D *vtxXY;
    TH2D *vtxVbdcX;
    TH2D *vtxVbdcY;
    TH1D *vtxHistZ;

    void Init();
    void DrawPID();
    void DrawTP();
    void DrawMul();
    void DrawVtxXY();
    void DrawVtxZ();

};

DiagnosticsGUI::DiagnosticsGUI(const TGWindow *p, UInt_t w, UInt_t h) {
    fMain = new TGMainFrame(p, w, h);

    // Make Tabs
    fTab = new TGTab(fMain, w, h);
    fTab->AddTab("PID");
    fTab->AddTab("ThetaPhi");
    fTab->AddTab("Multiplicity");
    fTab->AddTab("VertexXY");
    fTab->AddTab("VertexZ");
    fMain->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
    for(int i = 0; i < 5; i++) {
        auto name = TString::Format("Ecanvas%d", i);
        fEcanvas[i] = new TRootEmbeddedCanvas(name, fTab->GetTabContainer(i), 1280, 720);
        (fTab->GetTabContainer(i))->AddFrame(fEcanvas[i], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
    }

    // Make Bottom Frame
    TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain, 200, 40);

    // Run Input 
    TGLabel *runLabel = new TGLabel(hframe, "Run Number");
    hframe->AddFrame(runLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fRunDisp = new TGTextEntry(hframe);
    //eventNumber->SetAlignment(kTextRight);
    fRunDisp->SetEnabled(0);
    hframe->AddFrame(fRunDisp, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fRunEnt = new TGNumberEntry(hframe, 0.005, 9, 999, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, 0, 9999);
    fRunEnt->Connect("ValueSet(Int_t)", "DiagnosticsGUI", this, "SetRunNum()");
    (fRunEnt->GetNumberEntry())->Connect("ReturnPressed()", "DiagnosticsGUI", this, "SetRunNum()");
    hframe->AddFrame(fRunEnt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    // Button for processing run
    TGTextButton *process = new TGTextButton(hframe, "&Process");
    process->Connect("Clicked()", "DiagnosticsGUI", this, "ProcessRun()");
    hframe->AddFrame(process, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    // Exit button
    TGTextButton *exit = new TGTextButton(hframe,"&Exit", "gApplication->Terminate(0)");
    hframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    fMain->SetWindowName("TPC Diagnostics");
    fMain->MapSubwindows();
    fMain->Resize(fMain->GetDefaultSize());
    fMain->MapWindow();

    Init();
}

DiagnosticsGUI::~DiagnosticsGUI() {
    fMain->Cleanup();
    delete fMain;
}

void DiagnosticsGUI::ProcessRun() {

    TString filePath = TString::Format("data/run%04d_s00.reco.online.root", fRunNum);
    TFile *file = TFile::Open(filePath);
    if(!file || file->IsZombie()) {
        auto message = TString::Format("Error: Run %04d has not been unpacked! Unpack before plotting.", fRunNum);
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", message, kMBIconExclamation, kMBOk);
        if(file) {
            file->Close();
            delete file;
        }
        return;
    }
    file->Close();
    delete file;

    if(tree != nullptr) {
        delete tree;
        tree = nullptr;
    }
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    if(reader != nullptr) {
        delete reader;
        reader = nullptr;
    }
    reader = new TTreeReader(tree);

    if(vertexReader != nullptr) {
        delete vertexReader;
        vertexReader = nullptr;
    }
    vertexReader = new TTreeReaderValue<TClonesArray>(*reader, "STVertex");
    if(recoReader != nullptr) {
        delete recoReader;
        recoReader = nullptr;
    }
    recoReader = new TTreeReaderValue<TClonesArray>(*reader, "STRecoTrack");

    int eventCount = tree->GetEntries();

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        auto trackCount = recoReader->Get()->GetEntries();
        multiplicity->Fill(trackCount);
        vertexPtr = dynamic_cast<STVertex *>((*vertexReader)->At(0));

        //std::cout << i << std::endl;

        if(vertexPtr == NULL) {
            cout << "vertexPtr is null" << endl;
        }
        if(vertexPtr != NULL) {
            auto vertex = vertexPtr->GetPos();
            //cout << "vertex: (" << vertex.X() << ", " << vertex.Y() << ", " << vertex.Z() << ")" << endl;

            vtxXY->Fill(vertex.X(), vertex.Y() + 225);
            //vtxVbdcX->Fill(vertex.X(), fBDCProjection -> getX());
            //vtxVbdcY->Fill(vertex.Y(), fBDCProjection -> getY());
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
                //katanaPos->Fill(pos.X(), pos.Y());
                //cout << "track: " << r << "; mom: " << mom << "; dEdx: " << dedx << endl;

                auto momTar = recoPtr->GetMomentumTargetPlane();

                auto theta = momTar.Theta();
                auto phi = momTar.Phi();

                thetPhi->Fill(theta / pi * 180, (phi + pi) / pi * 180);
            }
        }
    }
    DrawPID();
    DrawTP();
    DrawMul();
    DrawVtxXY();
    DrawVtxZ();

    fRunDisp->SetText(TString::Format("%d", fRunNum));
}

void DiagnosticsGUI::Init() {

    pidHist = new TH2D("pidHist", "pidHist", 1000, -1000, 2000, 1000, 0, 1000);
    thetPhi = new TH2D("thetPhi", "thetPhi", 100, 0, 90, 100, 0, 360);
    multiplicity = new TH1I("multiplicity", "multiplicity", 100, 0, 100);

    vtxXY = new TH2D("vtxXY", "vtxXY", 100, -25, 25, 100, -25, 25);
    vtxVbdcX = new TH2D("vtxVbdcX", "vtxVbdcX", 100, -25, 25, 100, -25, 25);
    vtxVbdcY = new TH2D("vtxVbdcY", "vtxVbdcY", 100, -25, 25, 100, -25, 25);
    vtxHistZ = new TH1D("vtxHistZ", "vtxHistZ", 100, -100, 100);
}

void DiagnosticsGUI::DrawPID() {
    std::cout << "Drawing PID" << endl;
    TCanvas *canvas = fEcanvas[0]->GetCanvas();
    canvas->Clear();
    canvas->cd();
    pidHist->Draw("COLZ");
    canvas->Update();
}

void DiagnosticsGUI::DrawTP() {
    TCanvas *canvas = fEcanvas[1]->GetCanvas();
    canvas->Clear();
    canvas->cd();
    thetPhi->Draw("COLZ");
    canvas->Update();
}

void DiagnosticsGUI::DrawMul() {
    TCanvas *canvas = fEcanvas[2]->GetCanvas();
    canvas->Clear();
    canvas->cd();
    multiplicity->Draw();
    canvas->Update();
}

void DiagnosticsGUI::DrawVtxXY() {
    TCanvas *canvas = fEcanvas[3]->GetCanvas();
    canvas->Clear();
    canvas->Divide(2,2);
    canvas->cd(1);
    vtxXY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd();
    canvas->Update();
}

void DiagnosticsGUI::DrawVtxZ() {
    TCanvas *canvas = fEcanvas[4]->GetCanvas();
    canvas->Clear();
    canvas->cd();
    vtxHistZ->Draw();
    canvas->Update();
}



void diagGUI() {
    //TApplication theApp("App", 0, 0);

    new DiagnosticsGUI(gClient->GetRoot(), 200, 200);

    //theApp.Run();
}