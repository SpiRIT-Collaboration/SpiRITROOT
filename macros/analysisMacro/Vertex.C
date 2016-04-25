void Vertex() {
  
  auto fChain = new TChain("cbmsim");
  fChain -> Add("../data/runXXX.reco.root");

  TClonesArray *eventArray = NULL;
  fChain -> SetBranchAddress("STEvent", &eventArray);
  auto counter = 0;

  Int_t nEvents = fChain -> GetEntries();
  cout << "Number of events generated: " << nEvents << endl;

  TH2F* h1 = new TH2F("h1","Vertexing XY", 100, -3, 3, 100, -25, -18);
  h1->GetXaxis()->SetTitle("x (cm)");
  h1->GetYaxis()->SetTitle("y (cm)");
  h1->SetStats(0);
  TH1F* h2 = new TH1F("h2","Vertexing Z", 300, -20, 1);
  h2->GetXaxis()->SetTitle("z (cm)");
  h2->SetStats(0);

  // THIS NEEDS TO BE STUDIED IN DETAIL (i.e. dependent on RAVE parameters
  // and genfit particle hypothesis) - DO NOT TOUCH
  //  TVector3 correction(0.18, 0.28, 0.2);

  TCanvas* c1 = new TCanvas();
  TCanvas* c2 = new TCanvas();
  const Int_t kUPDATE = 20;
  
  for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) 
    {
      
      fChain -> GetEntry(iEvent);
      STEvent *event = (STEvent *) eventArray -> At(0);
      Int_t numVerticesFromEvent = event -> GetNumVertex();

      for (Int_t iVertex = 0; iVertex < numVerticesFromEvent; iVertex++) 
	{
	  ++counter;
	  genfit::GFRaveVertex *vertexFromEvent = (genfit::GFRaveVertex*) event -> GetVertex(iVertex);
	  Double_t x = vertexFromEvent -> getPos().X(); // - correction.X();
	  Double_t y = vertexFromEvent -> getPos().Y(); // - correction.Y();
	  Double_t z = vertexFromEvent -> getPos().Z(); // - correction.Z();
	  
	  h1->Fill(x, y);
	  h2->Fill(z);
	  
	}

      
      if (iEvent && (iEvent%kUPDATE) == 0) {
        if (iEvent == kUPDATE){ 
          c1->cd();
          h1->Draw("colz");
          c2->cd();
          h2->Draw();
        }
        c1->cd()->Modified();
        c1->cd()->Update();
        c2->cd()->Modified();
        c2->cd()->Update();
      }


    }

  cout << "Number of vertices reconstructed: " << counter << endl;
  
}
