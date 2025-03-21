TChain *tree = nullptr;

TTreeReader *reader = nullptr;
TTreeReaderValue<STAuxHeader> *auxHeaderReader = nullptr;
TTreeReaderValue<STEventHeader> *eventHeaderReader = nullptr;


void CheckReco(int runNum = 1635) {
    TString filePath = TString::Format("data_linktest/run%4d_s0*.reco.root", runNum);
    tree = new TChain("cbmsim");
    tree->Add(filePath);

    reader = new TTreeReader(tree);

    auxHeaderReader = new TTreeReaderValue<STAuxHeader>(*reader, "STAuxHeaderLinked");
    eventHeaderReader = new TTreeReaderValue<STEventHeader>(*reader, "STEventHeader");

    auto outFilename = TString::Format("recoTime%04d.txt", runNum);
    ofstream output;
    output.open(outFilename.Data());

    output << "EventNum  EventID  Time" << endl; 

    int eventCount = tree->GetEntries();

    for(int i = 0; i < eventCount; i++) {
        reader->Next();
        //cout << "event: " << i << endl;
        auto auxHeader = auxHeaderReader->Get();
        auto eventHeader = eventHeaderReader->Get();

        auto time = auxHeader->GetTpcTime();
        auto eventNum = auxHeader->GetTpcEventNum();
        auto eventID = eventHeader->GetEventID();
        auto bdcID = auxHeader->GetBdcID();

        output << eventNum << "  " << eventID << "  " << bdcID << "  " << time << "  " << endl;

    }

}
