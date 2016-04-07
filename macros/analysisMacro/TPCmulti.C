void TPCmulti() {

  auto fChain = new TChain("cbmsim");

  fChain -> Add("../data/filename.reco.root");

  TClonesArray *trackArray = NULL;
  fChain -> SetBranchAddress("STTrack", &trackArray);

  auto counter = 0;

  Int_t nEvents = fChain -> GetEntries();
  cout << "Number of events generated: " << nEvents << endl;

  TH1F* h1 = new TH1F("h1","TPC multiplicity", 100, 0,100);
  h1->GetXaxis()->SetTitle("multiplicity");
  h1->SetStats(0);

  TCanvas* c1 = new TCanvas();
  const Int_t kUPDATE = 20;
  
  for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) 
    {
      fChain -> GetEntry(iEvent);
      h1->Fill(trackArray -> GetEntries());
      
      if (iEvent && (iEvent%kUPDATE) == 0) {
	if (iEvent == kUPDATE) h1->Draw();
	c1->Modified();
	c1->Update();
      }
      
    }

}
