void TPCmulti() {

  auto fChain = new TChain("cbmsim");

  fChain -> Add("../data/runXXX.reco.root");

  TClonesArray *tracks = NULL;
  fChain -> SetBranchAddress("STTrack", &tracks);

  Int_t nEvents = fChain -> GetEntries();
  cout << "Number of events generated: " << nEvents << endl;

  TH1F* h1 = new TH1F("h1","TPC multiplicity", 100, 0,100);
  h1->GetXaxis()->SetTitle("multiplicity");
  h1->SetStats(0);

  TCanvas* c1 = new TCanvas();
  const Int_t kUPDATE = 20;

  Int_t numTracks;
  Int_t fittedTracks;
  for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) 
    {
      fChain -> GetEntry(iEvent);
      numTracks = tracks -> GetEntriesFast();
      fittedTracks = 0;
      for (Int_t iTrack = 0; iTrack < numTracks; iTrack++) {
	STTrack *track = (STTrack *) tracks -> At(iTrack);
	Double_t x = track->GetBeamVx()/10;
	Double_t y = track->GetBeamVy()/10;
	if (track -> IsFitted() && (x>-1.5 && x<1.5) && (y>-30 && y<-20))
	  fittedTracks++;
      }

      h1->Fill(fittedTracks);
      
      if (iEvent && (iEvent%kUPDATE) == 0) {
	if (iEvent == kUPDATE) h1->Draw();
	c1->Modified();
	c1->Update();
      }
      
    }

}
