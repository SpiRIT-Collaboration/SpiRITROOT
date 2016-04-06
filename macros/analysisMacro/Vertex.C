void Vertex() {
  
  auto fChain = new TChain("cbmsim");
  fChain -> Add("./data/filename.reco.root");

  TClonesArray *eventArray = NULL;
  fChain -> SetBranchAddress("STEvent", &eventArray);
  auto counter = 0;

  Int_t nEvents = fChain -> GetEntries();
  cout << "Number of events generated: " << nEvents << endl;

  TH2F* h1 = new TH2F("h1","Vertexing XY", 100, -3, 3, 100, -25, -18);
  TH1F* h2 = new TH1F("h2","Vertexing Z", 300, -20, 1);

  // THIS NEEDS TO BE STUDIED IN DETAIL (i.e. dependent on RAVE parameters
  // and genfit particle hypothesis) - DO NOT TOUCH
  TVector3 correction(0.18, 0.28, 0.2);
  
  for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) 
    {
      
      fChain -> GetEntry(iEvent);
      STEvent *event = (STEvent *) eventArray -> At(0);
      Int_t numVerticesFromEvent = event -> GetNumVertex();

      for (Int_t iVertex = 0; iVertex < numVerticesFromEvent; iVertex++) 
	{
	  ++counter;
	  genfit::GFRaveVertex *vertexFromEvent = (genfit::GFRaveVertex*) event -> GetVertex(iVertex);
	  Double_t x = vertexFromEvent -> getPos().X() - correction.X();
	  Double_t y = vertexFromEvent -> getPos().Y() - correction.Y();
	  Double_t z = vertexFromEvent -> getPos().Z() - correction.Z();
	  
	  h1->Fill(x, y);
	  h2->Fill(z);
	  
	}
    }

  cout << "Number of vertices reconstructed: " << counter << endl;
  
  TCanvas* c1 = new TCanvas();
  c1->cd();
  h1->GetXaxis()->SetTitle("x (cm)");
  h1->GetYaxis()->SetTitle("y (cm)");
  h1->SetStats(0);
  h1->Draw("colz");
  TLine* l1 = new TLine(-1.5,-19.33,1.5,-19.33);
  TLine* l2 = new TLine(1.5,-23.33,1.5,-19.33);
  TLine* l3 = new TLine(1.5,-23.33,-1.5,-23.33);
  TLine* l4 = new TLine(-1.5,-23.33,-1.5,-19.33);
  l1->SetLineColor(2);
  l1->SetLineWidth(4);
  l1->Draw("SAME");
  l2->SetLineColor(2);
  l2->SetLineWidth(4);
  l2->Draw("SAME");
  l3->SetLineColor(2);
  l3->SetLineWidth(4);
  l3->Draw("SAME");
  l4->SetLineColor(2);
  l4->SetLineWidth(4);
  l4->Draw("SAME");

  TCanvas* c2 = new TCanvas();
  c2->cd();
  h2->GetXaxis()->SetTitle("z (cm)");
  h2->SetStats(0);
  h2->Draw();

}
