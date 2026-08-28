void compareTimingBDC(int runNum) {

   ULong64_t eventTime;
   UInt_t eventID;
   UInt_t dataID;

   ULong64_t TimeStamp;

   ifstream metaList;
   //metaList.open(TString::Format("/data/RB230064/lokotko/reco_template/rawdataSupplement/run_%04d/metadataList.txt", runNum).Data());
   metaList.open(TString::Format("./metadata/run_%04d/metadataList.txt", runNum).Data());

   string filename;

   metaList >> filename;

   //TFile *f1 = new TFile(TString::Format("/data/RB230064/lokotko/reco_template/generateMetadata/run_%04d/metadata/*.C0.root", runNum).Data());
   //TFile *f1 = new TFile("/data/RB230064/lokotko/reco_template/generateMetadata/run_1242/metadata/*.C0.root");
   //TFile *f1 = new TFile(TString::Format("/data/RB230064/lokotko/reco_template/rawdataSupplement/run_%04d/%s", runNum, filename.c_str()).Data());
   TFile *f1 = new TFile(TString::Format("./metadata/run_%04d/%s", runNum, filename.c_str()).Data());
   
   auto tree = (TTree *) f1 -> Get("MetaData");
   if(!tree) {
      cout << "ERROR: MetaData tree is missing!" << endl;
      return;
   }
   tree -> SetBranchAddress("eventTime", &eventTime);
   tree -> SetBranchAddress("eventID", &eventID);
   tree -> SetBranchAddress("dataID", &dataID);

   TFile *f2 = new TFile(TString::Format("./bdc_files/bdc_%04d.root", runNum).Data());

   auto tree2 = (TTree *) f2 -> Get("TBDC"); 
   if(!tree2) {
      cout << "ERROR: TBDC tree is missing!" << endl;
      return;
   }
   tree2 -> SetBranchAddress("TimeStamp", &TimeStamp);

   TH1D *timeHist = new TH1D("timeHist", "timeHist", 60 * 40 , 0, 60 * 40);

   auto total = tree->GetEntries();
   auto rikenTot = tree2->GetEntries();

   cout << "total events: " << total / 4 << endl;
   cout << "total Riken events: " << rikenTot << endl;

   if(total / 4 > rikenTot) {
       cout << "Warning: More TPC events than RIKEN events!" << endl;
       //return;
   }
   
   //total = 10;

   tree->GetEntry(0);
   auto initTime = eventTime;
   auto initID = eventID;
   tree->GetEntry(total - 1);
   auto finalTime = eventTime;

   cout << "Avg Rate: " << total / 4. / (finalTime - initTime) / (10.0e-9) << endl;

    auto outFilename = TString::Format("timestampsBDC%04d.txt", runNum);
    ofstream output;
    output.open(outFilename.Data());

    output << "EventNum  EventID  MetaDataTime  RikenTime" << endl; 
  
   tree->GetEntry(0);
   tree2->GetEntry(0);
   int entryNum = 0;
   int initTimeStamp = TimeStamp;
   output << entryNum << "  " << eventID << "  " << eventTime - initTime << "  " << TimeStamp - initTimeStamp << endl;
   int prevEventID = eventID;
   for(int i = 1; i < total; i++) {
      tree->GetEntry(i);
      if(eventID != prevEventID + 1)
          continue;
      entryNum++;
      tree2->GetEntry(eventID - initID);
      output << eventID - initID << "  " << eventID << "  " << eventTime - initTime << "  " << TimeStamp - initTimeStamp << endl;
      prevEventID = eventID;
   }

    cout << "found events: " << entryNum  + 1 << endl;

    output.close();
      
}
