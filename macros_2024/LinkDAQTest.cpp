
ULong64_t eventTime; //metadata
UInt_t eventID; //metadata
ULong64_t TimeStamp; //bdc

TChain *fBdcTree;

UInt_t fBdcID = -1;

ULong64_t fBdcTimestamp = 0;
ULong64_t fTpcTimestamp = 0;
ULong64_t fOldBdcTimestamp = 0;
ULong64_t fOldTpcTimestamp = 0;

ULong64_t fBdcTreeIndex = 0;
ULong64_t fTpcTreeIndex = 0;

Double_t fSearchRadius = 1e-4;

Bool_t kFirstEvent = true;
Bool_t kMarkFill = true;

Int_t fEventCount = 0;

vector<ULong64_t> fBdcIntervals;
vector<ULong64_t> fTpcIntervals;

Bool_t fFatal = false;

ofstream fOutput;

Double_t GetScaledInterval(ULong64_t intervalEvt, ULong64_t fIntervalTPC)
{
    return static_cast<double>(fIntervalTPC) / static_cast<double>(intervalEvt);
}

void DoFirstEvent()
{
   fBdcTree->GetEntry(0);
   fBdcTimestamp = TimeStamp;
   fTpcTimestamp = eventTime;

   cout << "Initial timestamps: " << fTpcTimestamp << " " << fBdcTimestamp << endl;
   kFirstEvent = false;
   kMarkFill = false;
   fBdcTreeIndex = 1;
   fTpcTreeIndex = 1;
}

Int_t SyncStart()
{
   auto fIntervalBdc = fBdcTimestamp - fOldBdcTimestamp;
   auto fIntervalTpc = fTpcTimestamp - fOldTpcTimestamp;

   fBdcIntervals.push_back(fIntervalBdc);
   fTpcIntervals.push_back(fIntervalTpc);

   if(fTpcTreeIndex < 3)
      return -1;

   double scaledIntervals[2][2] = {};

   for(int i = 0; i < 2; i++) {
      for(int r = 0; r < 2; r++) {
         scaledIntervals[i][r] = GetScaledInterval(fBdcIntervals[r], fTpcIntervals[i]);
      }
   }

   if (scaledIntervals[0][0] - 1 < fSearchRadius && scaledIntervals[1][1] -1 < fSearchRadius) {
      if(!(scaledIntervals[0][0] < 1) && !(scaledIntervals[1][1] < 1)) {
         cout << "No DAQ offset found. Continuing DAQ Linking." << endl;
         return 0;
      }
   }

   if(scaledIntervals[1][0] - 1 < fSearchRadius && !(scaledIntervals[1][0] < 1)) {
      cout << "DAQ offset found. Extra TPC event at start." << endl;
      fBdcTreeIndex--;
      cout << "BDC index reduced by 1. Continuing DAQ Linking." << endl;
      fBdcTree->GetEntry(fBdcTreeIndex -1);
      fBdcTimestamp = TimeStamp;
      return 0;
   }

   if(scaledIntervals[0][1] - 1 < fSearchRadius && !(scaledIntervals[0][1] < 1)) {
      cout << "DAQ offset found. Extra BDC event at start." << endl;
      fBdcTreeIndex++;
      cout << "BDC index increased by 1. Continuing DAQ Linking." << endl;
      fBdcTree->GetEntry(fBdcTreeIndex - 1);
      fBdcTimestamp = TimeStamp;
      return 0;
   }

   cout << "FATAL: DAQ linking failed!" << endl;
   fFatal = true;
   return 1;
   
}

bool UpdateTimestamps()
{
   if (fBdcTree->GetEntry(fBdcTreeIndex) <= 0)
      return false;

   fBdcTreeIndex++;
   fTpcTreeIndex++;
   fOldBdcTimestamp = fBdcTimestamp;
   fOldTpcTimestamp = fTpcTimestamp;
   fBdcTimestamp = TimeStamp;
   fTpcTimestamp = eventTime;

   return true;
}

void ResetFlags()
{
   kMarkFill = true;
}



// return < 0 -> Extra bdc event
// return == 0 -> match
// return > 0 -> extra TPC
Int_t CheckMatch()
{
    auto  fIntervalBdc = fBdcTimestamp - fOldBdcTimestamp;
    auto fIntervalTpc = fTpcTimestamp - fOldTpcTimestamp;

    //|intervalTPC-intervalRIBF|/freqRatio
    auto scaledInterval = GetScaledInterval(fIntervalBdc, fIntervalTpc);

    if (scaledInterval - 1 < fSearchRadius && !(scaledInterval - 1 < 0))
       return 0;

    cout << "TPC Timestamp: " << fTpcTimestamp << std::endl
              << "EVT Timestamp: " << fBdcTimestamp << std::endl
              << "TPC Interval: " << fIntervalTpc << std::endl
              << "EVT Interval: " << fIntervalBdc << std::endl
              << "Scaled Interval: " << scaledInterval - 1 << std::endl;

    if (scaledInterval > 1)
       return -1;
    else
       return 1;
}

void Exec()
{
   ResetFlags();
   // Should already have loaded the event from iomanager. If this is the
   // first event then set the old timestamp and continue without filling

   if (kFirstEvent) {
      DoFirstEvent();
      return;
   }

   // Grab both timestamps for this event, set old timestamp and update counter
   if (!UpdateTimestamps()) {
      cout << "Failed to update timestamps. Skipping event" << endl;
      kMarkFill = false;
      return;
   }

   // Compare both timestamps with their old ones. If it is negative, we must
   // have taken an event before the clocks cleared so reset the old TS
   // and continue without filling
   if (fBdcTimestamp < fOldBdcTimestamp ||
       fTpcTimestamp < fOldTpcTimestamp) {
      cout << "Mismatch! reseting old timestamps and continuing." << endl;
      cout << "EVT TS old: " << fOldBdcTimestamp << " EVT TS new: " << fBdcTimestamp << endl;
      cout << "TPC TS old: " << fOldTpcTimestamp
                << " TPC TS new: " << fTpcTimestamp << endl;
      kMarkFill = false;
      return;
   }

   if(fTpcTreeIndex < 4) {
      SyncStart();
      return;
   }

   // Compare intervalTPC/intervalNSCL to the search mean and radius
   // If it lies within, then fill the HiRAET tree, upadate the old timestamps,
   // and return
   auto match = CheckMatch();
   // LOG(info) << match;
   if (match == 0) {
      // Record timestamps
      fBdcID = fBdcTreeIndex - 1;
      return;
   }

   /***** If we have an extra EVT event *****/
   cout << "Extra event, match number: " << match << endl;
   if (match < 0) {

      cout << "Extra EVT event found, searching for matching event" << endl;

      // Loop through until we hit the last entry of the tree, or until
      // we can match the event interval,
      auto currentIndex = fBdcTreeIndex;
      auto maxIndex = currentIndex + 3;
      // auto currentTS = fEvtTimestamp;
      if (maxIndex > fBdcTree->GetEntries())
         maxIndex = fBdcTree->GetEntries();

      // Change to a loop until EVT timestamp is further along
      while (currentIndex < maxIndex) {
         // Get the next NSCL event and associated interval
         fBdcTree->GetEntry(currentIndex++);
         fBdcTimestamp = TimeStamp;

         // Check for a match
         if (CheckMatch() == 0) {
            cout << "Found match!" << std::endl;
            fBdcTreeIndex = currentIndex;
            fBdcID = fBdcTreeIndex - 1;

            return;
         }
      } // while(currentIndex < maxIndex)

      cout << "Did not find match in next three events" << std::endl;
      kMarkFill = false;

   } // if we had an extra EVT event

   /**** We have an extra TPC event *******/
   else {
      cout << "Extra TPC  event found, searching for matching event!" << endl;

      kMarkFill = false;
      // Reset the evtIndex so we're still looking at the same event
      fBdcTreeIndex--;
      fBdcTree->GetEntry(fBdcTreeIndex -1);
      fBdcTimestamp = TimeStamp;
      return;
   }
}

void LinkDAQTest(int runNum) {
   ifstream metaList;
   metaList.open(TString::Format("./metadata/run_%04d/metadataList.txt", runNum).Data());

   string filename;

   metaList >> filename;

   TFile *f1 = new TFile(TString::Format("./metadata/run_%04d/%s", runNum, filename.c_str()).Data());
   
   auto tree = (TTree *) f1 -> Get("MetaData");
   if(!tree) {
      cout << "ERROR: MetaData tree is missing!" << endl;
      return;
   }
   tree -> SetBranchAddress("eventTime", &eventTime);
   tree -> SetBranchAddress("eventID", &eventID);

   auto bdcFilename = TString::Format("./bdc_files/bdc_%04d.root", runNum);

   fBdcTree = new TChain("TBDC"); 
   fBdcTree->Add(bdcFilename);
   if(!fBdcTree) {
      cout << "ERROR: TBDC tree is missing!" << endl;
      return;
   }
   fBdcTree -> SetBranchAddress("TimeStamp", &TimeStamp);

   auto total = tree->GetEntries();
   auto rikenTot = fBdcTree->GetEntries();

   total = 40;

   /*if(total / 4 > rikenTot) {
       cout << "FATAL: More TPC events than RIKEN events!" << endl;
       return;
   }*/

    auto outFilename = TString::Format("linked_TpcCut%04d.txt", runNum);
    fOutput.open(outFilename.Data());

    fOutput << "EventNum  EventID  BDCID  MetaDataTime  RikenTime" << endl; 

   tree->GetEntry(0);
   int prevEventID = eventID - 1;
   int initEventID = eventID;
   for(int i = 0; i < total; i++) {
      if(fFatal)
         return;
      tree->GetEntry(i);
      if(eventID != prevEventID + 1)
          continue;

     if(eventID - initEventID == 5) {
          prevEventID = eventID;
          cout << "\"removing\" TPC event 5" << endl;
          continue;
      }

     /*if(eventID - initEventID == 5) {
          fBdcTreeIndex++;
          cout << "\"removing\" BDC event 10" << endl;
      }*/

      Exec();
      if(kMarkFill)
         fOutput << eventID - initEventID << "  " << eventID << "  " << fBdcID << "  " << eventTime << "  " << TimeStamp << endl;
      prevEventID = eventID;
   }

   fOutput.close();

}