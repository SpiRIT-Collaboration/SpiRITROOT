void driftT(){

  /////////////////////
  // load the file  
  /////////////////////
  TChain *chain = new TChain("cbmsim");
  chain->Add("/home/giordano/ShareWindows/run0928_skimmed.reco.root");

  TClonesArray* eventArray = NULL;
  chain-> SetBranchAddress("STEvent", &eventArray);

  // output histogram
  TH1F* h1 = new TH1F("h1","time bucket dist",512,0,511);

  Int_t nentries = chain->GetEntries();
  std::cout << "Num. events: " << nentries << std::endl;

  for(int i=0 ; i < nentries ; i++){
    chain -> GetEntry(i);
    
    STEvent *fEvent;
    fEvent = (STEvent*)eventArray->At(0);
    Int_t nhits = fEvent -> GetNumHits();

    if (fEvent->IsGood()==kFALSE)
      continue;

    for(int j=0 ; j<nhits ; j++){
      
    STHit *hit = fEvent->GetHit(j);
    TVector3 vect = (TVector3)hit->GetPosition();
    Double_t xpos = vect.X();
    Double_t zpos = vect.Z();
    Double_t ypos = vect.Y();
    Double_t tb = hit->GetTb(); 
 
    h1->Fill(tb);
    }
  }

  TCanvas* c1 = new TCanvas("c1","",700,500);
  c1->cd();
  c1->SetLogy();
  h1->GetXaxis()->SetTitle("time buckets");
  h1->Draw();

}

