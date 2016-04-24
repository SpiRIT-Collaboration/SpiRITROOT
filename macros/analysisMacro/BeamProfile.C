void BeamProfile() {

  auto fChain = new TChain("cbmsim");

  fChain -> Add("../data/runXXX.reco.root");

  TClonesArray *trackArray = NULL;
  fChain -> SetBranchAddress("STTrack", &trackArray);

  auto counter = 0;

  Int_t nEvents = fChain -> GetEntries();
  cout << "Number of events generated: " << nEvents << endl;

  TH2F* h1 = new TH2F("h1","Beam XY", 100, -3, 3, 100, -25, -18);
  h1->GetXaxis()->SetTitle("x (cm)");
  h1->GetYaxis()->SetTitle("y (cm)");
  h1->SetStats(0);
  TH2F* h2 = new TH2F("h2","Katana Hit Map", 200, -40., 40., 200, -50, 0);
  h2->GetXaxis()->SetTitle("x (cm)");
  h2->GetYaxis()->SetTitle("y (cm)");
  h2->SetStats(0);
  TH1F* h3 = new TH1F("h3","Angle XZ", 200,-15,15);
  h3->GetXaxis()->SetTitle("angle XZ (degrees)");
  h3->SetStats(0);
  TH1F* h4 = new TH1F("h4","Angle YZ", 200,-15,15);
  h4->GetXaxis()->SetTitle("angle YZ (degrees)");
  h4->SetStats(0);

  TCanvas* c1 = new TCanvas("c1","",800,800);
  c1->Divide(2,2);
  const Int_t kUPDATE = 10;
  
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

	  if (trackFromArray -> IsFitted()){
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
	    
	    h2->Fill(-xK, yK);
	  }
	  
	}
      
      if (iEvent && (iEvent%kUPDATE) == 0) {
        if (iEvent == kUPDATE){ 
	  c1->cd(1);
	  h1->Draw("colz");
	  c1->cd(2);
	  h2->Draw("colz");
	  c1->cd(3);
	  h3->Draw();
	  c1->cd(4);
	  h4->Draw();
	}
        c1->cd(1)->Modified();
        c1->cd(1)->Update();
        c1->cd(2)->Modified();
        c1->cd(2)->Update();
        c1->cd(3)->Modified();
        c1->cd(3)->Update();
        c1->cd(4)->Modified();
        c1->cd(4)->Update();
      }
      
      
    }

  c1->cd(1);
  h1->Draw("colz");
  c1->cd(2);
  h2->Draw("colz");
  c1->cd(3);
  h3->Draw();
  c1->cd(4);
  h4->Draw();

  c1->cd(1)->Modified();
  c1->cd(1)->Update();
  c1->cd(2)->Modified();
  c1->cd(2)->Update();
  c1->cd(3)->Modified();
  c1->cd(3)->Update();
  c1->cd(4)->Modified();
  c1->cd(4)->Update();

}
