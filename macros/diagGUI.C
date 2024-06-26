#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <TGFileDialog.h>
#include <RQ_OBJECT.h>

const double pi = 3.1415926;

class DiagnosticsGUI {
    RQ_OBJECT("DiagnosticsGUI")
public:
    DiagnosticsGUI(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~DiagnosticsGUI();

    void SetRunNum() { fRunNum = fRunEnt->GetNumberEntry()->GetIntNumber(); }
    void SetClustCut() { fClustCut = fCutEnt->GetNumberEntry()->GetIntNumber(); }
    void SetPOCACut() { fPOCACut = fPOCAEnt->GetNumberEntry()->GetNumber(); }
    void SetVertZPCut() { fVertZPosCut = fVertZPosEnt->GetNumberEntry()->GetNumber(); }
    void SetVertZSCut() { fVertZSigCut = fVertZSigEnt->GetNumberEntry()->GetNumber(); }
    void SetCurrentTab(Int_t tab) { fCurrentTab = tab; }
    void ProcessRun();
    void ToggleEditor();
    void PopOutCanvas();

private:
    TGMainFrame *fMain;
    TRootEmbeddedCanvas *fEcanvas[10];
    TGTab *fTab;
    TGTextEntry *fRunDisp;
    TGNumberEntry *fRunEnt;
    TGNumberEntry *fCutEnt;
    TGNumberEntry *fPOCAEnt;
    TGNumberEntry *fVertZPosEnt;
    TGNumberEntry *fVertZSigEnt;
    TGCheckButton *fVertZCheck;
    TGCheckButton *fTargetCheck;
    TGCheckButton *fOnlineCheck;

    Int_t fRunNum = 1045;
    Int_t fClustCut = 15;
    Int_t fCurrentTab = 0;
    Double_t fPOCACut = 20;
    Double_t fVertZPosCut = -27.7;
    Double_t fVertZSigCut = 1.9;
    Double_t fTargetXWidth = 30;
    Double_t fThreshold = 5;

    TCanvas *cPop = nullptr;

    TChain *tree = nullptr;

    TTreeReader *reader = nullptr;
    TTreeReaderValue<TClonesArray> *vertexReader = nullptr;
    TTreeReaderValue<TClonesArray> *recoReader = nullptr;
    TTreeReaderValue<TClonesArray> *rawEventReader = nullptr;

    STVertex *vertexPtr;
    STRecoTrack *recoPtr;
    STRawEvent *rawEventPtr;

    TH2D *pidHist;
    TH2D *thetPhi;
    TH1I *multiplicity;

    TH2D *vtxXY;
    TH2D *bdcXY;
    TH2D *tbdcXY;
    TH2D *vtxVbdcX;
    TH2D *vtxVbdcY;
    TH2D *vtxVtbdcX;
    TH2D *vtxVtbdcY;
    TH1D *vtxHistZ;
    TH1D *vtxX;
    TH1D *vtxY;

    TH2D *tbdcVbdcX;
    TH2D *tbdcVbdcY;
    TH1D *bdcX;
    TH1D *bdcY;
    TH1D *bdcA;
    TH1D *bdcB;
    TH1D *tbdcX;
    TH1D *tbdcY;
    TH1D *tbdcA;
    TH1D *tbdcB;

    TH2D *beamHist;
    TH1D *beamEnd; 
    TH1D *beamMid; 

    void Init();
    void DrawPID(Int_t tab);
    void DrawTP(Int_t tab);
    void DrawMul(Int_t tab);
    void DrawVtxXY(Int_t tab);
    void DrawVtxXY2(Int_t tab);
    void DrawBDC(Int_t tab);
    void DrawBDC2(Int_t tab);
    void DrawVtxZ(Int_t tab);
    void DrawBeam(Int_t tab);

};

DiagnosticsGUI::DiagnosticsGUI(const TGWindow *p, UInt_t w, UInt_t h) {
    fMain = new TGMainFrame(p, w, h);
    TGHorizontalFrame *hPlots = new TGHorizontalFrame(fMain, 200, 40);

    TGVerticalFrame *vSidebar =  new TGVerticalFrame(hPlots, 200, 40);
    // Make Cut Frame
    TGHorizontalFrame *hCut = new TGHorizontalFrame(vSidebar, 200, 40);

    TGLabel *cutLabel = new TGLabel(hCut, "Cluster Cut");
    hCut->AddFrame(cutLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fCutEnt = new TGNumberEntry(hCut, 0.005, 9, 999, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 9999);
    fCutEnt->Connect("ValueSet(Int_t)", "DiagnosticsGUI", this, "SetClustCut()");
    (fCutEnt->GetNumberEntry())->Connect("ReturnPressed()", "DiagnosticsGUI", this, "SetClustCut()");
    fCutEnt->SetNumber(fClustCut);
    hCut->AddFrame(fCutEnt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    vSidebar->AddFrame(hCut, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGHorizontalFrame *hPOCA = new TGHorizontalFrame(vSidebar, 200, 40);
    TGLabel *pocaLabel = new TGLabel(hPOCA, "POCA Cut");
    hPOCA->AddFrame(pocaLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fPOCAEnt = new TGNumberEntry(hPOCA, 0.005, 9, 999, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 9999);
    fPOCAEnt->Connect("ValueSet(Int_t)", "DiagnosticsGUI", this, "SetPOCACut()");
    (fPOCAEnt->GetNumberEntry())->Connect("ReturnPressed()", "DiagnosticsGUI", this, "SetPOCACut()");
    fPOCAEnt->SetNumber(fPOCACut);
    hPOCA->AddFrame(fPOCAEnt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    vSidebar->AddFrame(hPOCA, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

   // Frame for using a vertex Z cut
    TGVerticalFrame *vVertZ = new TGVerticalFrame(vSidebar, 200, 40);
    TGHorizontalFrame *hVertZP = new TGHorizontalFrame(vVertZ, 200, 40);
    TGLabel *vertZPLabel = new TGLabel(hVertZP, "Vertex Z Position");
    hVertZP->AddFrame(vertZPLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fVertZPosEnt = new TGNumberEntry(hVertZP, 0.005, 9, 999, 
                                     TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, -9999, 9999);
    fVertZPosEnt->Connect("ValueSet(Int_t)", "DiagnosticsGUI", this, "SetVertZPCut()");
    (fVertZPosEnt->GetNumberEntry())->Connect("ReturnPressed()", "DiagnosticsGUI", this, "SetVertZPCut()");
    fVertZPosEnt->SetNumber(fVertZPosCut);
    hVertZP->AddFrame(fVertZPosEnt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    vVertZ->AddFrame(hVertZP, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGHorizontalFrame *hVertZS = new TGHorizontalFrame(vVertZ, 200, 40);
    TGLabel *vertZSLabel = new TGLabel(hVertZS, "Vertex Z Sigma");
    hVertZS->AddFrame(vertZSLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fVertZSigEnt = new TGNumberEntry(hVertZS, 0.005, 9, 999, 
                                     TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 9999);
    fVertZSigEnt->Connect("ValueSet(Int_t)", "DiagnosticsGUI", this, "SetVertZSCut()");
    (fVertZSigEnt->GetNumberEntry())->Connect("ReturnPressed()", "DiagnosticsGUI", this, "SetVertZSCut()");
    fVertZSigEnt->SetNumber(fVertZSigCut);
    hVertZS->AddFrame(fVertZSigEnt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    vVertZ->AddFrame(hVertZS, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    fVertZCheck = new TGCheckButton(vVertZ, "Use Vertex Z Cut", 1);
    fVertZCheck->SetState(kButtonUp);
    vVertZ->AddFrame(fVertZCheck, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    vSidebar->AddFrame(vVertZ, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGHorizontalFrame *hTarget = new TGHorizontalFrame(vSidebar, 200, 40);
    fTargetCheck = new TGCheckButton(hTarget, "Use Target XY Cut", 1);
    fTargetCheck->SetState(kButtonUp);
    hTarget->AddFrame(fTargetCheck, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    vSidebar->AddFrame(hTarget, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    hPlots->AddFrame(vSidebar, new TGLayoutHints(kLHintsLeft, 5, 5, 3, 4));


    // Make Tabs
    fTab = new TGTab(hPlots, w, h);
    fTab->AddTab("PID");
    fTab->AddTab("ThetaPhi");
    fTab->AddTab("Multiplicity");
    fTab->AddTab("VertexXY");
    //fTab->AddTab("VertexXY2");
    fTab->AddTab("BDC");
    //fTab->AddTab("BDCangles");
    fTab->AddTab("VertexZ");
    //fTab->AddTab("BeamTrack");
    fTab->Connect("Selected(Int_t)", "DiagnosticsGUI", this, "SetCurrentTab(Int_t)");
    hPlots->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
    for(int i = 0; i < 6; i++) {
        //TGHorizontalFrame *htopbar = new TGHorizontalFrame(fTab->GetTabContainer(i), 200, 40 );
        //TGTextButton *edit = new TGTextButton(htopbar, "&Editor");
        //edit->Connect("Clicked()", "DiagnosticsGUI", this, "ToggleEditor()");
        //htopbar->AddFrame(edit, new TGLayoutHints(kLHintsLeft, 5, 5, 3, 4));
        //(fTab->GetTabContainer(i))->AddFrame(htopbar, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5));
        auto name = TString::Format("Ecanvas%d", i);
        fEcanvas[i] = new TRootEmbeddedCanvas(name, fTab->GetTabContainer(i), 1280, 720);
        (fTab->GetTabContainer(i))->AddFrame(fEcanvas[i], new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
    }
    fMain->AddFrame(hPlots, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    // Make Bottom Frame
    TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain, 200, 40);

    TGTextButton *popOut = new TGTextButton(hframe, "&Pop Out");
    popOut->Connect("Clicked()", "DiagnosticsGUI", this, "PopOutCanvas()");
    hframe->AddFrame(popOut, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    fOnlineCheck = new TGCheckButton(hframe, "Online", 1);
    fOnlineCheck->SetState(kButtonUp);
    hframe->AddFrame(fOnlineCheck, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

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
   pidHist->Reset();
   thetPhi->Reset();
   multiplicity->Reset();
             
   vtxXY->Reset();
   vtxVbdcX->Reset();
   vtxVbdcY->Reset();
   vtxVtbdcX->Reset();
   vtxVtbdcY->Reset();
   vtxHistZ->Reset();

   bdcXY->Reset();
   tbdcXY->Reset();
   tbdcVbdcX->Reset();
   tbdcVbdcY->Reset();
   vtxX->Reset();
   vtxY->Reset();

   bdcX->Reset();
   bdcY->Reset();
   bdcA->Reset();
   bdcB->Reset();
   tbdcX->Reset();
   tbdcY->Reset();
   tbdcA->Reset();
   tbdcB->Reset();

   TString format;
   if(fOnlineCheck->IsDown())
      format = "reco.online";
   else
      format = "reco.2024";

   TString testPath = TString::Format("data/run%04d_s00.%s.root", fRunNum, format.Data());
   TFile *file = TFile::Open(testPath);
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

   TString filePath = TString::Format("data/run%04d_s*.%s.root", fRunNum, format.Data());

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
    if(rawEventReader != nullptr) {
        delete rawEventReader;
        rawEventReader = nullptr;
    }
    rawEventReader = new TTreeReaderValue<TClonesArray>(*reader, "STRawEvent");


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

    TString fBeamData = Form("/data/s063/exp/exp2406_s063/anaroot/users/shift/bdc_rootfiles/bdc_%d.root", fRunNum);
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
        auto trackCount = recoReader->Get()->GetEntries();
        multiplicity->Fill(trackCount);

        /*rawEventPtr = dynamic_cast<STRawEvent *>((*rawEventReader)->At(0));
        if(rawEventPtr == NULL) {
            cout << "rawEventPtr is null" << endl;
        }
        if(rawEventPtr != NULL) {
            for(int layer = 0; layer < 112; layer++) {
                for(int row = 0; row < 60; row++) {
                    auto pad = rawEventPtr->GetPad(row, layer);
                    auto adc = pad->GetADC();
                    double maxADC = 0;
                    for(int r = 0; r < 512 / 2; r++) {
                        if(pad->GetADC(r) > maxADC) {
                            maxADC = pad->GetADC(r);
                            if(maxADC > fThreshold)
                                break;
                        }
                    }
                    //cout << maxADC << endl;

                    if(maxADC < fThreshold) {
                        auto currVal = beamHist->GetBinContent(layer + 1, row + 1);
                        beamHist->SetBinContent(layer + 1, row + 1, currVal + 1);
                        if(layer == 111)
                           beamEnd->Fill(row);
                        if(layer == 83)
                           beamMid->Fill(row);
                    }
                }
            }
        }*/

        vertexPtr = dynamic_cast<STVertex *>((*vertexReader)->At(0));

        TVector3 vertex;

        if(vertexPtr == NULL) {
            //cout << "vertexPtr is null" << endl;
            continue;
        }
        if(vertexPtr != NULL) {
            vertex = vertexPtr->GetPos();
            //cout << "vertex: (" << vertex.X() << ", " << vertex.Y() << ", " << vertex.Z() << ")" << endl;

            vtxHistZ->Fill(vertex.Z());

            if(fVertZCheck->IsDown() && (vertex.Z() < fVertZPosCut - fVertZSigCut * 3 || vertex.Z() > fVertZPosCut + fVertZSigCut * 3))
               continue;

            vtxXY->Fill(vertex.X(), vertex.Y() + 240);
            if(useBDC) {
               //Double_t ProjectedAtZ = -580.4 - 27.7;  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
               Double_t ProjectedAtZ = -589 - (27.7 - (592.644 - 580.4));  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
               //fBDCProjection -> ProjectParticle(bdc_x, bdc_y, -5667.52, bdc_a, bdc_y, 124. / 54., 377.2 * 124, ProjectedAtZ, 123.90589);//-580.4,-583.904
               //auto pBDC_X = fBDCProjection->getX();
               //auto pBDC_Y = fBDCProjection->getY();
               //auto pBDC_A = fBDCProjection->getA();
               //auto pBDC_B = fBDCProjection->getB();

               //vtxVbdcX->Fill(vertex.X(), pBDC_X);
               //vtxVbdcY->Fill(vertex.Y() + 240, pBDC_Y);
               vtxVtbdcX->Fill(vertex.X(), tbdc_x);
               vtxVtbdcY->Fill(vertex.Y() + 240, tbdc_y);
               vtxX->Fill(vertex.X());
               vtxY->Fill(vertex.Y() + 240);

               //tbdcVbdcX->Fill(tbdc_x, pBDC_X);
               //tbdcVbdcY->Fill(tbdc_y, pBDC_Y);
               //bdcXY->Fill(pBDC_X, pBDC_Y);
               //bdcX->Fill(pBDC_X);
               //bdcY->Fill(pBDC_Y);
               //bdcA->Fill(pBDC_A);
               //bdcB->Fill(pBDC_B);
               tbdcXY->Fill(tbdc_x, tbdc_y);
               tbdcX->Fill(tbdc_x);
               tbdcY->Fill(tbdc_y);
               tbdcA->Fill(tbdc_a);
               tbdcB->Fill(tbdc_b);
            }
        }

        if(fVertZCheck->IsDown() && (vertex.Z() < fVertZPosCut - fVertZSigCut * 3 || vertex.Z() > fVertZPosCut + fVertZSigCut * 3))
            continue;
        if(fTargetCheck->IsDown() && (vertex.X() < -fTargetXWidth / 2. || vertex.X() > fTargetXWidth / 2.))
            continue;

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

                  //if(charge != gfCharge)
                     //cout << "charges were different" << endl;

                  pidHist->Fill(mom * gfCharge, dedx);

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
    }
    DrawPID(0);
    DrawTP(1);
    DrawMul(2);
    //DrawVtxXY(3);
    DrawVtxXY2(3);
    //DrawBDC2(5);
    DrawBDC(4);
    DrawVtxZ(5);
    //DrawBeam(6);

    fRunDisp->SetText(TString::Format("%d", fRunNum));
}

void DiagnosticsGUI::Init() {

    pidHist = new TH2D("pidHist", "pidHist", 1000, -500, 2000, 1000, 0, 1000);
    pidHist->SetTitle("PID");
    pidHist->GetXaxis()->SetTitle("p/Z (MeV/c)");
    pidHist->GetXaxis()->CenterTitle();
    pidHist->GetYaxis()->SetTitle("dE/dx (ADC/mm)");
    pidHist->GetYaxis()->CenterTitle();
    thetPhi = new TH2D("thetPhi", "thetPhi", 100, 0, 90, 100, 0, 360);
    thetPhi->SetTitle("Theta vs Phi");
    thetPhi->GetXaxis()->SetTitle("theta (deg)");
    thetPhi->GetXaxis()->CenterTitle();
    thetPhi->GetYaxis()->SetTitle("phi (deg)");
    thetPhi->GetYaxis()->CenterTitle();
    multiplicity = new TH1I("multiplicity", "multiplicity", 100, 0, 100);
    multiplicity->SetTitle("Track Multiplicity");
    multiplicity->GetXaxis()->SetTitle("number of tracks");
    multiplicity->GetXaxis()->CenterTitle();
    multiplicity->GetYaxis()->SetTitle("counts");
    multiplicity->GetYaxis()->CenterTitle();

    vtxXY = new TH2D("vtxXY", "vtxXY", 200, -50, 50, 200, -50, 50);
    vtxXY->SetTitle("Vertex position");
    vtxXY->GetXaxis()->SetTitle("X (mm)");
    vtxXY->GetXaxis()->SetTitleSize(0.05);
    vtxXY->GetXaxis()->SetLabelSize(0.05);
    vtxXY->GetXaxis()->CenterTitle();
    vtxXY->GetYaxis()->SetTitle("Y (mm)");
    vtxXY->GetYaxis()->SetTitleSize(0.05);
    vtxXY->GetYaxis()->SetLabelSize(0.05);
    vtxXY->GetYaxis()->CenterTitle();
    bdcXY = new TH2D("bdcXY", "bdcXY", 200, -50, 50, 200, -50, 50);
    bdcXY->SetTitle("BDC projection to target");
    bdcXY->GetXaxis()->SetTitle("X (mm)");
    bdcXY->GetXaxis()->SetTitleSize(0.05);
    bdcXY->GetXaxis()->SetLabelSize(0.05);
    bdcXY->GetXaxis()->CenterTitle();
    bdcXY->GetYaxis()->SetTitle("Y (mm)");
    bdcXY->GetYaxis()->SetTitleSize(0.05);
    bdcXY->GetYaxis()->SetLabelSize(0.05);
    bdcXY->GetYaxis()->CenterTitle();
    tbdcXY = new TH2D("tbdcXY", "tbdcXY", 200, -50, 50, 200, -50, 50);
    tbdcXY->SetTitle("BDC projection to target");
    tbdcXY->GetXaxis()->SetTitle("X (mm)");
    tbdcXY->GetXaxis()->SetTitleSize(0.05);
    tbdcXY->GetXaxis()->SetLabelSize(0.05);
    tbdcXY->GetXaxis()->CenterTitle();
    tbdcXY->GetYaxis()->SetTitle("Y (mm)");
    tbdcXY->GetYaxis()->SetTitleSize(0.05);
    tbdcXY->GetYaxis()->SetLabelSize(0.05);
    tbdcXY->GetYaxis()->CenterTitle();
    vtxVbdcX = new TH2D("vtxVbdcX", "vtxVbdcX", 200, -50, 50, 200, -50, 50);
    vtxVbdcX->SetTitle("Vertex X vs BDC projection X");
    vtxVbdcX->GetXaxis()->SetTitle("Vertex X (mm)");
    vtxVbdcX->GetXaxis()->SetTitleSize(0.05);
    vtxVbdcX->GetXaxis()->SetLabelSize(0.05);
    vtxVbdcX->GetXaxis()->CenterTitle();
    vtxVbdcX->GetYaxis()->SetTitle("BDC X (mm)");
    vtxVbdcX->GetYaxis()->SetTitleSize(0.05);
    vtxVbdcX->GetYaxis()->SetLabelSize(0.05);
    vtxVbdcX->GetYaxis()->CenterTitle();
    vtxVbdcY = new TH2D("vtxVbdcY", "vtxVbdcY", 200, -50, 50, 200, -50, 50);
    vtxVbdcY->SetTitle("Vertex Y vs BDC projection Y");
    vtxVbdcY->GetXaxis()->SetTitle("Vertex Y (mm)");
    vtxVbdcY->GetXaxis()->SetTitleSize(0.05);
    vtxVbdcY->GetXaxis()->SetLabelSize(0.05);
    vtxVbdcY->GetXaxis()->CenterTitle();
    vtxVbdcY->GetYaxis()->SetTitle("BDC Y (mm)");
    vtxVbdcY->GetYaxis()->SetTitleSize(0.05);
    vtxVbdcY->GetYaxis()->SetLabelSize(0.05);
    vtxVbdcY->GetYaxis()->CenterTitle();
    vtxVtbdcX = new TH2D("vtxVtbdcX", "vtxVtbdcX", 200, -50, 50, 200, -50, 50);
    vtxVtbdcX->SetTitle("Vertex X vs BDC projection X");
    vtxVtbdcX->GetXaxis()->SetTitle("Vertex X (mm)");
    vtxVtbdcX->GetXaxis()->SetTitleSize(0.05);
    vtxVtbdcX->GetXaxis()->SetLabelSize(0.05);
    vtxVtbdcX->GetXaxis()->CenterTitle();
    vtxVtbdcX->GetYaxis()->SetTitle("BDC X (mm)");
    vtxVtbdcX->GetYaxis()->SetTitleSize(0.05);
    vtxVtbdcX->GetYaxis()->SetLabelSize(0.05);
    vtxVtbdcX->GetYaxis()->CenterTitle();
    vtxVtbdcY = new TH2D("vtxVtbdcY", "vtxVtbdcY", 200, -50, 50, 200, -50, 50);
    vtxVtbdcY->SetTitle("Vertex Y vs BDC projection Y");
    vtxVtbdcY->GetXaxis()->SetTitle("Vertex Y (mm)");
    vtxVtbdcY->GetXaxis()->SetTitleSize(0.05);
    vtxVtbdcY->GetXaxis()->SetLabelSize(0.05);
    vtxVtbdcY->GetXaxis()->CenterTitle();
    vtxVtbdcY->GetYaxis()->SetTitle("BDC Y (mm)");
    vtxVtbdcY->GetYaxis()->SetTitleSize(0.05);
    vtxVtbdcY->GetYaxis()->SetLabelSize(0.05);
    vtxVtbdcY->GetYaxis()->CenterTitle();
    vtxHistZ = new TH1D("vtxHistZ", "vtxHistZ", 1000, -500, 800);
    vtxHistZ->SetTitle("Vertex Z Position");
    vtxHistZ->GetXaxis()->SetTitle("Vertex Z (mm)");
    //vtxHistZ->GetXaxis()->SetTitleSize(0.05);
    //vtxHistZ->GetXaxis()->SetLabelSize(0.05);
    vtxHistZ->GetXaxis()->CenterTitle();
    vtxHistZ->GetYaxis()->SetTitle("Counts");
    //vtxHistZ->GetYaxis()->SetTitleSize(0.05);
    //vtxHistZ->GetYaxis()->SetLabelSize(0.05);
    vtxHistZ->GetYaxis()->CenterTitle();
    vtxX = new TH1D("vtxX", "vtxX", 200, -50, 50);
    vtxX->SetTitle("Vertex X Position");
    vtxX->GetXaxis()->SetTitle("Vertex X (mm)");
    vtxX->GetXaxis()->SetTitleSize(0.05);
    vtxX->GetXaxis()->SetLabelSize(0.05);
    vtxX->GetXaxis()->CenterTitle();
    vtxX->GetYaxis()->SetTitle("Counts");
    vtxX->GetYaxis()->SetTitleSize(0.05);
    vtxX->GetYaxis()->SetLabelSize(0.05);
    vtxX->GetYaxis()->CenterTitle();
    vtxY = new TH1D("vtxY", "vxtY", 200, -50, 50);
    vtxY->SetTitle("Vertex Y Position");
    vtxY->GetXaxis()->SetTitle("Vertex Y (mm)");
    vtxY->GetXaxis()->SetTitleSize(0.05);
    vtxY->GetXaxis()->SetLabelSize(0.05);
    vtxY->GetXaxis()->CenterTitle();
    vtxY->GetYaxis()->SetTitle("Counts");
    vtxY->GetYaxis()->SetTitleSize(0.05);
    vtxY->GetYaxis()->SetLabelSize(0.05);
    vtxY->GetYaxis()->CenterTitle();

    tbdcVbdcX = new TH2D("tbdcVbdcX", "tbdcVbdcX", 200, -50, 50, 200, -50, 50);
    tbdcVbdcY = new TH2D("tbdcVbdcY", "tbdcVbdcY", 200, -50, 50, 200, -50, 50);
    bdcX = new TH1D("bdcX", "bdcX", 200, -50, 50);
    bdcY = new TH1D("bdcY", "bdcY", 200, -50, 50);
    bdcA = new TH1D("bdcA", "bdcA", 100, -3.14, 3.14);
    bdcB = new TH1D("bdcB", "bdcB", 100, -3.14, 3.14);
    tbdcX = new TH1D("tbdcX", "tbdcX", 200, -50, 50);
    tbdcX->SetTitle("BDC Projection X");
    tbdcX->GetXaxis()->SetTitle("BDC projection X (mm)");
    tbdcX->GetXaxis()->SetTitleSize(0.05);
    tbdcX->GetXaxis()->SetLabelSize(0.05);
    tbdcX->GetXaxis()->CenterTitle();
    tbdcX->GetYaxis()->SetTitle("Counts");
    tbdcX->GetYaxis()->SetTitleSize(0.05);
    tbdcX->GetYaxis()->SetLabelSize(0.05);
    tbdcX->GetYaxis()->CenterTitle();
    tbdcY = new TH1D("tbdcY", "tbdcY", 200, -50, 50);
    tbdcY->SetTitle("BDC Projection Y");
    tbdcY->GetXaxis()->SetTitle("BDC projection Y (mm)");
    tbdcY->GetXaxis()->SetTitleSize(0.05);
    tbdcY->GetXaxis()->SetLabelSize(0.05);
    tbdcY->GetXaxis()->CenterTitle();
    tbdcY->GetYaxis()->SetTitle("Counts");
    tbdcY->GetYaxis()->SetTitleSize(0.05);
    tbdcY->GetYaxis()->SetLabelSize(0.05);
    tbdcY->GetYaxis()->CenterTitle();
    tbdcA = new TH1D("tbdcA", "tbdcA", 100, -3.14, 3.14);
    tbdcB = new TH1D("tbdcB", "tbdcB", 100, -3.14, 3.14);

    beamHist = new TH2D("beamHist", "beamHist", 112, 0, 1344, 108, -432, 432);

    beamEnd = new TH1D("beamEnd", "beamEnd", 108, -432, 432);
    beamMid = new TH1D("beamMid", "beamMid", 108, -432, 432);
}

void DiagnosticsGUI::DrawPID(Int_t tab) {
    std::cout << "Drawing PID" << endl;
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    canvas->cd();
    gStyle->SetPalette(55);
    pidHist->Draw("COLZ");
   //pidHist->Draw("");
    canvas->Update();
}

void DiagnosticsGUI::DrawTP(Int_t tab) {
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    canvas->cd();
    thetPhi->Draw("COLZ");
    canvas->Update();
}

void DiagnosticsGUI::DrawMul(Int_t tab) {
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    canvas->cd();
    multiplicity->Draw();
    canvas->Update();
}

void DiagnosticsGUI::DrawVtxXY(Int_t tab) {
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    canvas->Divide(2,2);
    canvas->cd(1);
    vtxXY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(2);
    bdcXY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(3);
    vtxVbdcX->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(4);
    vtxVbdcY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd();
    canvas->Update();
}

void DiagnosticsGUI::DrawVtxXY2(Int_t tab) {
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    canvas->Divide(2,2);
    canvas->cd(1);
    vtxXY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(2);
    tbdcXY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(3);
    vtxVtbdcX->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(4);
    vtxVtbdcY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd();
    canvas->Update();
}

void DiagnosticsGUI::DrawBDC(Int_t tab) {
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    canvas->Divide(2,2);
    canvas->cd(1);
    vtxX->Draw();
    gPad->Modified();
    canvas->Update();
    canvas->cd(2);
    vtxY->Draw();
    gPad->Modified();
    canvas->Update();
    canvas->cd(3);
    tbdcX->Draw();
    gPad->Modified();
    canvas->Update();
    canvas->cd(4);
    tbdcY->Draw();
    gPad->Modified();
    canvas->Update();
    /*canvas->cd(5);
    bdcX->Draw();
    gPad->Modified();
    canvas->Update();
    canvas->cd(6);
    bdcY->Draw();
    gPad->Modified();
    canvas->Update();*/
    canvas->cd();
    canvas->Update();
}

void DiagnosticsGUI::DrawBDC2(Int_t tab) {
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    canvas->Divide(2,2);
    canvas->cd(1);
    tbdcXY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(2);
    bdcXY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(3);
    tbdcVbdcX->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd(4);
    tbdcVbdcY->Draw("COLZ");
    gPad->Modified();
    canvas->Update();
    canvas->cd();
    canvas->Update();
}

void DiagnosticsGUI::DrawVtxZ(Int_t tab) {
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    canvas->cd();
    vtxHistZ->Draw();
    canvas->Update();
}

void DiagnosticsGUI::DrawBeam(Int_t tab) {
    TCanvas *canvas = fEcanvas[tab]->GetCanvas();
    canvas->Clear();
    beamHist->Draw("COLZ");
    canvas->Update();
}

 void DiagnosticsGUI::ToggleEditor() {
    TCanvas *canvas = fEcanvas[fCurrentTab]->GetCanvas();
    //canvas->ToggleEditor();
    canvas->EditorBar();
 }

 void DiagnosticsGUI::PopOutCanvas() {
    TCanvas *canvas = fEcanvas[fCurrentTab]->GetCanvas();
    if(cPop != nullptr ) {
       delete cPop;
       cPop = nullptr;
    }
    cPop = new TCanvas("cPop", "cPop", 1);
    cPop->cd();
    canvas->DrawClonePad();
    //cPop->Update();
 }

void diagGUI() {
    //TApplication theApp("App", 0, 0);

    new DiagnosticsGUI(gClient->GetRoot(), 200, 200);

    //theApp.Run();
}
