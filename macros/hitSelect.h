namespace hitSelect
{
    FairRootManager *fRootManager;

    TClonesArray *fRawHitArray = nullptr;
    TClonesArray *fHitArray = nullptr;

    Bool_t fIsPersistence = kFALSE;
    TString fInputBranchName = "STHitRaw";
    TString fOutputBranchName = "STHit";

    Double_t fBoundry = 252;

    void Init();
    void Exec();
    void SetPersistence(Bool_t value = kTRUE){ fIsPersistence = value; };

    void SetInputBranch(TString name){ fInputBranchName = name; };
    void SetOutputBranch(TString name){ fOutputBranchName = name; };

    void SetBoundry(Double_t value){ fBoundry = value; };


    void Init()
    {
        fRootManager = FairRootManager::Instance();
        if (fRootManager == nullptr) {
            cout << "Cannot find RootManager!" << endl;
            return;
        }

        fRawHitArray = (TClonesArray *) fRootManager -> GetObject(fInputBranchName);
        if (fRawHitArray == nullptr) {
            cout << "Cannot find " << fInputBranchName << "array!" << endl;
            return;
        }

        fHitArray = new TClonesArray("STHit", 1000);
        fRootManager -> Register(fOutputBranchName, "SpiRIT", fHitArray, fIsPersistence);
    }

    void Exec()
    {
        for(int i = 0; i < fRawHitArray -> GetEntriesFast(); i++) {
            auto hit = dynamic_cast<STHit *>(fRawHitArray->At(i));
            if(hit == nullptr) {
                return;
            }
            auto pos = hit -> GetPosition();
            if(pos.X() > -432 + fBoundry && pos.X() < 432 - fBoundry && pos.Z() > fBoundry && pos.Z() < 1344 - fBoundry) {
                new ((*fHitArray)[fHitArray->GetEntriesFast()]) STHit(hit);
            } 
        }
    }
}