void pid_plot(
  bool drawPIDLine = true,
  STPID::PID selectPID = STPID::kNon, // kPion, kProton, kDeuteron, kTriton, k3He, k4He, KNon
  Int_t nbins = 500,
  Int_t p1 = -1000,
  Int_t p2 = 1200,
  Int_t dedx1 = 0,
  Int_t dedx2 = 800
)
{

  gStyle -> SetPadBottomMargin(0.10);
  gStyle -> SetPadLeftMargin(0.11);
  gStyle -> SetPadRightMargin(0.12);
  gStyle -> SetTitleFontSize(0.06);

  auto tree = new TChain("cbmsim");
  tree -> AddFile("data/mc0_s0.reco.v1.04.root");//m0.reco.");
    
  TClonesArray *trackArray = nullptr;
  TClonesArray *vertexArray = nullptr;

  tree -> SetBranchAddress("STRecoTrack", &trackArray);
  tree -> SetBranchAddress("STVertex", &vertexArray);

  auto hVertexXY = new TH2D("hVertexXY",";x (mm);y (mm)",100,-100,100,100,-300,-150);
  auto hVertexZY = new TH1D("hVertexZY",";z (mm)",2000,-100,1344);

  auto histPID = new TH2D("histPID",";p/q (MeV/c/z); dEdx (ADC/mm)",nbins,p1,p2,nbins,dedx1,dedx2);
  histPID -> SetTitle("All events");
  histPID -> SetTitleSize(0.04,"xy");
  histPID -> SetTitleOffset(1.4,"y");
  histPID -> SetTitleOffset(1.1,"x");
  histPID -> GetXaxis() -> CenterTitle();
  histPID -> GetYaxis() -> CenterTitle();
  
  Int_t nEvents = tree -> GetEntries();
  for (Int_t iEvent = 0; iEvent < nEvents; ++iEvent)
  {
    double ndf = 0;
    double dist = 0;
    double vx =0, vy =0, vz =0;
    int vid =0;
    
    if (iEvent % 500 == 0)
      cout << "Event " << iEvent << endl;

    tree -> GetEntry(iEvent);

    Int_t nTracks = trackArray -> GetEntries();
    for (auto iTrack = 0; iTrack < nTracks; ++iTrack) {

      auto track = (STRecoTrack *) trackArray -> At(iTrack);
      auto pid =  track -> GetPID();
      auto q =   track  -> GetCharge();
      auto p =   track  -> GetMomentum().Mag()/q;
      auto dedx = track -> GetdEdxWithCut(0,0.7);
      ndf = track -> GetClusterIDArray() -> size(); 

      auto parentvid = track -> GetVertexID();

      if (parentvid < 0) {
	vid = -99998;
	vx = -99998;
	vy = -99998;
	vz = -99998;
	dist = -99998;
      } else {
	vid = parentvid;
	auto vertex = (STVertex *) vertexArray -> At(parentvid);
	auto posVertex = vertex -> GetPos();

	vx = posVertex.X();
	vy = posVertex.Y();
	vz = posVertex.Z();

	auto pocaVertex = track -> GetPOCAVertex();
	dist = (pocaVertex - posVertex).Mag();
      }

      bool vertex_cut = (vz<-9.49569 && vz>-12.80121 && vx>-15 && vx<15 && vy<-206.06&& vy>-246.06);//good vertex for data
      bool all_cut = (vertex_cut && dist<5 && ndf>30);

      //      if (all_cut==false)
	//	continue;

      histPID -> Fill(p, dedx);
    }
  }
  
  TFile *out = new TFile("MC_hist.root","RECREATE");
  auto cvsPID = new TCanvas("cvsPID","",20,20,1200,700);
  histPID->Write();
  cvsPID->SetLogz();
  histPID -> Draw("colz"); 
  //  cvsPID->SaveAs("histPID.png");
  
}
