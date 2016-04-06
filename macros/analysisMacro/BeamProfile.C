void BeamProfile() {

  auto fChain = new TChain("cbmsim");

  fChain -> Add("./data/filename.reco.root");

  TClonesArray *trackArray = NULL;
  fChain -> SetBranchAddress("STTrack", &trackArray);


  auto counter = 0;

  Int_t nEvents = fChain -> GetEntries();
  cout << "Number of events generated: " << nEvents << endl;

  TH2F* h1 = new TH2F("h1","Beam XY", 100, -3, 3, 100, -25, -18);
  TH2F* h2 = new TH2F("h2","Katana Hit Map", 200, -40., 40., 200, -50, 0);
  TH1F* h3 = new TH1F("h3","Angle XZ", 200,-15,15);
  TH1F* h4 = new TH1F("h4","Angle YZ", 200,-15,15);
  
  for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) 
    {
      
      fChain -> GetEntry(iEvent);
      Int_t numTracksFromArray = trackArray -> GetEntries();

      TVector3 recopos(0,0,0);
      TVector3 recop(0,0,0);
      TMatrixDSym covv(6,6);
      
      for (Int_t iTrack = 0; iTrack < numTracksFromArray; iTrack++) 
	{

	  STTrack *trackFromArray = (STTrack*) trackArray -> At(iTrack);

	  //////////////////////////////
	  // beam at target position
	  //////////////////////////////
	  Double_t x = trackFromArray->GetBeamVx()/10;
	  Double_t y = trackFromArray->GetBeamVy()/10;
	  Double_t z = trackFromArray->GetBeamVz()/10;

	  Double_t px = trackFromArray->GetBeamMomentum().X();
	  Double_t py = trackFromArray->GetBeamMomentum().Y();
	  Double_t pz = trackFromArray->GetBeamMomentum().Z();

	  h1->Fill(x, y);

	  // beam exit angles
	  Double_t angle_1 = TMath::ATan(px/pz);
	  Double_t angle_2 = TMath::ATan(py/pz);

	  h3->Fill(angle_1);
	  h4->Fill(angle_2);

	  //////////////////////////////
	  // beam at katana position
	  //////////////////////////////
	  Double_t xK = trackFromArray->GetKatanaHitX()/10;
          Double_t yK = trackFromArray->GetKatanaHitY()/10;
          Double_t zK = trackFromArray->GetKatanaHitZ()/10;

	  h2->Fill(xK, yK);


	}
    }

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
  h2->GetXaxis()->SetTitle("x (cm)");
  h2->GetYaxis()->SetTitle("y (cm)");
  h2->SetStats(0);
  h2->Draw("colz");

  TCanvas* c3 = new TCanvas();
  c3->cd();
  h3->GetXaxis()->SetTitle("angle XZ (degrees)");
  h3->SetStats(0);
  h3->Draw();

  TCanvas* c4 = new TCanvas();
  c4->cd();
  h4->GetXaxis()->SetTitle("angle YZ (degrees)");
  h4->SetStats(0);
  h4->Draw();

}
