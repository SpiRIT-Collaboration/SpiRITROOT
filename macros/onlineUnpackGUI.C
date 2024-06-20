#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <RQ_OBJECT.h>

class UnpackGUI {
    RQ_OBJECT("UnpackGUI")
public:
    UnpackGUI(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~UnpackGUI();

    void SetRunNum() { fRunNum = fRunEnt->GetNumberEntry()->GetIntNumber(); }
    void SetNumEvents() { fNumEvents = fEventsEnt->GetNumberEntry()->GetIntNumber(); }
    void UnpackRun();

    void UnpackFinish();

private:
    TGMainFrame *fMain;
    TGNumberEntry *fRunEnt;
    TGNumberEntry *fEventsEnt;
    TGLabel *fStatusText;
    TGLabel *fUnpackStatus;
    TGTextButton *fUnpack;

    Int_t fRunNum = 0;
    Int_t fNumEvents = 1000;

    const TString fStatusReady = "Ready to unpack!";
    const TString fStatusBusy = "Unpacking in progress. Please wait.";
    const TString fOutForm = "reco.online";

    bool CheckUnpack();

    void SetBusy();
    void SetReady();

};

UnpackGUI::UnpackGUI(const TGWindow *p, UInt_t w, UInt_t h) {
    fMain = new TGMainFrame(p, w, h);

    // Run Input Frame
    TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain, 200, 40);

    // Run Input 
    TGLabel *runLabel = new TGLabel(hframe, "Run Number");
    hframe->AddFrame(runLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fRunEnt = new TGNumberEntry(hframe, 0.005, 9, 999, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, 0, 9999);
    fRunEnt->Connect("ValueSet(Int_t)", "UnpackGUI", this, "SetRunNum()");
    (fRunEnt->GetNumberEntry())->Connect("ReturnPressed()", "UnpackGUI", this, "SetRunNum()");
    hframe->AddFrame(fRunEnt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    // Events Input Frame
    TGHorizontalFrame *hEframe = new TGHorizontalFrame(fMain, 200, 40);

    // Events Input 
    TGLabel *eventsLabel = new TGLabel(hEframe, "Events");
    hEframe->AddFrame(eventsLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fEventsEnt = new TGNumberEntry(hEframe, 0.005, 9, 999, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 9999);
    fEventsEnt->SetIntNumber(fNumEvents);
    fEventsEnt->Connect("ValueSet(Int_t)", "UnpackGUI", this, "SetNumEvents()");
    (fEventsEnt->GetNumberEntry())->Connect("ReturnPressed()", "UnpackGUI", this, "SetNumEvents()");
    hEframe->AddFrame(fEventsEnt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fMain->AddFrame(hEframe, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    // Status 
    TGHorizontalFrame *hStatusFrame = new TGHorizontalFrame(fMain, 200, 40);
    TGGroupFrame *fStatusGroup = new TGGroupFrame(hStatusFrame, "Status", kVerticalFrame);
    fStatusText = new TGLabel(fStatusGroup, fStatusBusy);
    fStatusGroup->AddFrame(fStatusText, new TGLayoutHints(kLHintsLeft, 5, 5, 3, 4));
    fUnpackStatus = new TGLabel(fStatusGroup, "");
    fStatusGroup->AddFrame(fUnpackStatus, new TGLayoutHints(kLHintsLeft, 5, 5, 3, 4));
    hStatusFrame->AddFrame(fStatusGroup, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fMain->AddFrame(hStatusFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    // Buttons Frame
    TGHorizontalFrame *hUframe = new TGHorizontalFrame(fMain, 200, 40);
    // Button for processing run
    fUnpack = new TGTextButton(hUframe, "&Unpack");
    fUnpack->Connect("Clicked()", "UnpackGUI", this, "UnpackRun()");
    hUframe->AddFrame(fUnpack, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    // Exit button
    TGTextButton *exit = new TGTextButton(hUframe,"&Exit", "gApplication->Terminate(0)");
    hUframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
    fMain->AddFrame(hUframe, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    fMain->SetWindowName("TPC Unpack");
    fMain->MapSubwindows();
    fMain->Resize(fMain->GetDefaultSize());
    fMain->MapWindow();

    gSystem->ProcessEvents();
    fStatusText->SetText(fStatusReady);

}

UnpackGUI::~UnpackGUI() {
    fMain->Cleanup();
    delete fMain;
}

void UnpackGUI::UnpackRun() {
    fUnpack->SetEnabled(false);
    fRunEnt->SetState(false);
    fEventsEnt->SetState(false);
    fStatusText->SetText(fStatusBusy);
    fStatusText->Resize();
    gClient->NeedRedraw(fStatusText);

    ofstream parlFile;
    parlFile.open("input/parInpt.txt");

    for(int i = 4; i < 9; i++) {

      auto macroRunner = TString::Format("root -b -l -q \"run_reco_2024.C(%d, %d, %d, \\\"%s\\\")\"", fRunNum, i, fNumEvents, fOutForm.Data());

      parlFile << macroRunner.Data() << std::endl;
    }
    parlFile.close();

    auto parlRunner = TString::Format("./parallel --jobs 5 --joblog ./log.txt < input/parInpt.txt");

    fUnpackStatus->SetText(TString::Format("Unpacking Run %04d", fRunNum));
    fUnpackStatus->Resize();
    gClient->NeedRedraw(fUnpackStatus);

    gSystem->ProcessEvents();

    int exitCode = gSystem->Exec(parlRunner.Data());

    if (exitCode == 0) {
        fUnpackStatus->SetText("Unpack status: Finished");
    } else {
        fUnpackStatus->SetText("Unpack status: Error");
        std::cout << "Exit Code: " << exitCode << std::endl;
    }
    fUnpackStatus->Resize();
    fUnpackStatus->Layout();

    UnpackFinish();
}

void UnpackGUI::UnpackFinish() {
    CheckUnpack();

    SetReady();
}

bool UnpackGUI::CheckUnpack() {
    TString filePath = TString::Format("data/run%04d_s00.%s.root", fRunNum, fOutForm.Data());
    TFile *file = TFile::Open(filePath);
    if(!file || file->IsZombie()) {
        auto message = TString::Format("Error: Run %04d did not unpack! No File!", fRunNum);
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", message, kMBIconExclamation, kMBOk);
        if(file) {
            file->Close();
            delete file;
        }
        return false;
    }
    TTree *tree = (TTree*)file->Get("cbmsim");
    if(tree == nullptr) {
        auto message = TString::Format("Error: Run %04d did not unpack correctly! No TTree cbmsim!", fRunNum);
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", message, kMBIconExclamation, kMBOk);
        return false;
    }
    if(tree->GetEntries() != fNumEvents) {
        auto message = TString::Format("Warning: Run %04d unpacked but with %lld events!", fRunNum, tree->GetEntries());
        new TGMsgBox(gClient->GetRoot(), fMain, "Warning", message, kMBIconExclamation, kMBOk);
        if(tree->GetEntries() == 0)
            return false;
        return true;
    }

    file->Close();
    delete file;

    auto message = TString::Format("Run %04d unpacked successfully!", fRunNum);
    new TGMsgBox(gClient->GetRoot(), fMain, "Success", message, kMBIconExclamation, kMBOk);

    return true;
}

void UnpackGUI::SetBusy() {
}

void UnpackGUI::SetReady() {
    fUnpack->SetEnabled(true);
    fRunEnt->SetState(true);
    fEventsEnt->SetState(true);
    fStatusText->SetText(fStatusReady);
    fStatusText->Resize();
    fStatusText->Layout();
}

void onlineUnpackGUI() {
    //TApplication theApp("App", 0, 0);

    new UnpackGUI(gClient->GetRoot(), 200, 200);

    //theApp.Run();
}
