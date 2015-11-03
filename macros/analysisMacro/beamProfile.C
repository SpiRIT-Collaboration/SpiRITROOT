#include <vector>

void beamProfile(){

  /////////////////////////////////////////////////////////////////////////////////////
  // List of parameters (if you don't what you are doing don't touch the code below) //
  /////////////////////////////////////////////////////////////////////////////////////

  Double_t fWindowTbStart = 70;        // start value of the tb window
  Double_t fTBTime = 40;               // sampling rate
  Double_t fDriftVelocity = 5.43/100;  // drift velocity
  Double_t yPosition = -21.33;         // y-position of the target
  Double_t zPosition = -7.31;          // z-position of the target
  Int_t trkcntr = 0;                   // counter for tracks coming from the vertex

  /////////////////////////////////////////////////////////////////////////////////////

  ////////////////////
  // load the file  
  ////////////////////
  TChain *chain = new TChain("cbmsim");
  chain->Add("/home/giordano/ShareWindows/run0928_skimmed.reco.root");

  ////////////////////
  // load branches
  ////////////////////  
  TClonesArray* fLinearTrackArray = NULL;
  chain-> SetBranchAddress("STLinearTrack", &fLinearTrackArray);
  TClonesArray* eventArray = NULL;
  chain-> SetBranchAddress("STEvent", &eventArray);

  ////////////////////
  // output histogram
  ////////////////////
  TH1F* h1 = new TH1F("h1","beam X",50,-10.,10.);
  TH1F* h2 = new TH1F("h2","beam Y",50,-30.33,-12.33);
  TH2F* h3 = new TH2F("h3","beam profile",30,-10.,10.,30,-25.33,-17.33);

  ///////////////////////////////////////
  // stuff that you don't want to touch
  ///////////////////////////////////////
  STLinearTrackFitter* fLTFitter = new STLinearTrackFitter();

  Int_t nentries = chain->GetEntries();

  Double_t fWindowYStart = fWindowTbStart * fTBTime * fDriftVelocity / 10.;

  
  // loop over all events  
  for(int i=0 ; i < nentries ; i++){
    chain -> GetEntry(i);
    
    STEvent *fEvent;
    fEvent = (STEvent*)eventArray->At(0);
    STLinearTrack* track;
    Int_t nTracks = fLinearTrackArray -> GetEntries();

    std::cout << "Event: " << i << endl;
    std::cout << "NTracks: " << nTracks << std::endl;

    for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) {
      track = (STLinearTrack*) fLinearTrackArray -> At(iTrack);
      Int_t nHits = track -> GetNumHits();

      TVector3 position = fLTFitter -> GetPointOnZ(track, zPosition);
      Double_t new_y = position.Y()/10.;
      new_y += fWindowYStart;

      // debug 
      //      std::cout << -position.X()/10. << " " << new_y << " " << position.Z() << std::endl;   
      
      h1->Fill(-position.X()/10.);
      h2->Fill(new_y);
      h3->Fill(-position.X()/10.,new_y);

      if (TMath::Abs(position.X()/10.) < 3 && TMath::Abs(new_y - yPosition) < 4)
	trkcntr++;
    }
  }   
  std::cout << "Num. events: " << nentries << std::endl;
  std::cout << "Number of good tracks coming from the target: " << trkcntr << std::endl;
  std::cout << "Avg. good tracks/event: " << double(trkcntr)/nentries << std::endl;

  ///////////////////
  // plotting stuff
  ///////////////////

  TCanvas* c1 = new TCanvas("c1","",700,500);
  c1->cd();
  h1->SetStats(0);
  h1-> GetXaxis() -> SetTitle("x (cm)");
  h1->Draw();

  TCanvas* c2 = new TCanvas("c2","",700,500);
  c2->cd();
  h2->SetStats(0);
  h2-> GetXaxis() -> SetTitle("y (cm)");
  h2->Draw();

  TCanvas* c3 = new TCanvas("c3","",700,500);
  c3->cd();
  h3-> SetMarkerStyle(29);
  h3-> SetMarkerSize(6);
  h3-> GetXaxis() -> SetTitle("x (cm)");
  h3-> GetXaxis() -> CenterTitle();
  h3-> GetYaxis() -> SetTitle("y (cm)");
  h3-> GetYaxis() -> CenterTitle();
  h3-> SetMinimum(0);
  //  h3-> SetStats(0);
  h3-> Draw("COLZ");
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

}

