namespace waves
{
FairRootManager *fRootManager;
TClonesArray *fRawEventArray = nullptr;
TH1D *hist = new TH1D("hist", "hist", 250, -250, 0);
TString outFile = "data/outFile.root";

void Init() {
    fRootManager = FairRootManager::Instance();
    if (fRootManager == nullptr) {
        cout << "Cannot find RootManager!" << endl;
    }

    fRawEventArray = (TClonesArray *) fRootManager -> GetObject("STRawEvent");
    if (fRawEventArray == nullptr) {
        cout << "Cannot find STRawEvent array!" << endl;
    }
}

void Exec() {
    STRawEvent *rawEvent = (STRawEvent *) fRawEventArray -> At(0);
    //do whatever with histogram to fill it here
}

void Finalize() {
    TFile *f1 = new TFile(outFile.Data(), "RECREATE");
    hist->Write();
}

}