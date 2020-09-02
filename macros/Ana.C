Double_t proton_mass = 938.272; // proton
Double_t deuteron_mass = 1875.61; // deuteron
Double_t triton_mass = 2808.92; // triton
Double_t pion_mass = 139.57; // pi+-

Double_t beamangle132 = -44.06/1000;// rad, Sn-132
Double_t beamangle108 = -55.19/1000;// rad, Sn-108

Bool_t isEmbed = kTRUE;
//Bool_t isEmbed = kFALSE;
TString outfile = "tree";

void Ana(TString dstlist="dstlist.txt",
	 TString outfilename="",
	 TString mclist="mclist.txt",
	 int neventana = -1) {


  
  Double_t mass = pion_mass;
  if(dstlist.Contains("proton")){
    mass = proton_mass;
    outfile += "_proton_";
    std::cout << "this is proton analysis. mass = " << mass << std::endl;
  }
  else if(dstlist.Contains("deuteron")){
    mass = deuteron_mass;
    outfile += "_deuteron_";
    std::cout << "this is deuteron analysis. mass = " << mass << std::endl;
  }
  else if(dstlist.Contains("triton")){
    mass = triton_mass;
    outfile += "_triton_";
    std::cout << "this is triton analysis. mass = " << mass << std::endl;
  }
  else{
    outfile += "_pion_";
  }

  Double_t beamangle = 0;
  if(dstlist.Contains("embed132")){
    isEmbed = kTRUE;
    outfile += "embed132.root";
    beamangle = beamangle132;
    std::cout << "this is embed132 analysis." << std::endl;
  }
  else if(dstlist.Contains("embed108")){
    isEmbed = kTRUE;
    outfile += "embed108.root";
    beamangle = beamangle108;
    std::cout << "this is embed108 analysis." << std::endl;
  }
  else if(dstlist.Contains("single")){
    isEmbed = kFALSE;
    outfile += "single.root";
    std::cout << "this is single analysis." << std::endl;
  }
  else{
    outfile += ".root";
  }

  if(!(outfilename.IsNull()))
    outfile = outfilename;

  ifstream inf(dstlist.Data());
  std::string dname;
  auto fChain = new TChain("cbmsim");
  while(inf>>dname){
    std::cout << dname << std::endl;
    fChain -> Add(dname.c_str());
  }
  inf.close();

  /*
  ifstream mcinf(mclist.Data());
  auto mcChain = new TChain("cbmsim");
  while(mcinf>>dname){
    std::cout << dname << std::endl;
    mcChain -> Add(dname.c_str());
  }
  mcinf.close();
  */

  TClonesArray *mcArray = NULL;
  //  mcChain -> SetBranchAddress("PrimaryTrack", &mcArray);
  fChain -> SetBranchAddress("STMCTrack", &mcArray);
  TClonesArray *vtrkArray = NULL;
  TClonesArray *rtrkArray = NULL;
  if(isEmbed){
    //fChain -> SetBranchAddress("VATracks", &vtrkArray);
    fChain -> SetBranchAddress("EmbedVATracks", &vtrkArray);
    //fChain -> SetBranchAddress("STEmbedRecoTrack", &vtrkArray);
  }
  else{
    fChain -> SetBranchAddress("STEmbedRecoTrack", &vtrkArray);
  }
  //fChain -> SetBranchAddress("STRecoTrack", &rtrkArray);
  fChain -> SetBranchAddress("STEmbedRecoTrack", &rtrkArray);
  TClonesArray *vtxArray = NULL;
  fChain -> SetBranchAddress("STVertex", &vtxArray);
  STEventHeader *eventHeader = NULL;
  fChain -> SetBranchAddress("STEventHeader", &eventHeader);

  Int_t nEvents = fChain -> GetEntries();
  cout << "Number of events generated: " << nEvents << endl;

  Int_t ntrk, ngrtrk, nrtrk, nmctrk, netrk, nuetrk, ngetrk, ngeetrk, nggeetrk, nmggeetrk, nvtx, nvtxtrk, ggclose;
  Double_t vtxx, vtxy, vtxz, aoq, zet;
  cout << "Making the output tree file of: " << outfile.Data() << endl;
  TFile *fout = new TFile(outfile.Data(),"recreate");
  TTree *evetr = new TTree("evetree","event tree");
  evetr->Branch("ntrk",  &ntrk,  "ntrk/I");
  evetr->Branch("ngrtrk",  &ngrtrk,  "ngrtrk/I");
  evetr->Branch("nrtrk",  &nrtrk,  "nrtrk/I");
  evetr->Branch("nmctrk",  &nmctrk,  "nmctrk/I");
  evetr->Branch("netrk", &netrk, "netrk/I");
  evetr->Branch("nuetrk", &nuetrk, "nuetrk/I");
  evetr->Branch("ngetrk", &ngetrk, "ngetrk/I");
  evetr->Branch("ngeetrk", &ngeetrk, "ngeetrk/I");
  evetr->Branch("nggeetrk", &nggeetrk, "nggeetrk/I");
  evetr->Branch("nmggeetrk", &nmggeetrk, "nmggeetrk/I");
  evetr->Branch("nvtx",  &nvtx,  "nvtx/I");
  evetr->Branch("vtxx",  &vtxx,  "vtxx/D");
  evetr->Branch("vtxy",  &vtxy,  "vtxy/D");
  evetr->Branch("vtxz",  &vtxz,  "vtxz/D");
  evetr->Branch("aoq",  &aoq,  "aoq/D");
  evetr->Branch("zet",  &zet,  "zet/D");
  evetr->Branch("ggclose",  &ggclose,  "ggclose/I");

  Int_t pdg;
  Double_t mccmpx, mccmpy, mccmpz, mccmp, mccme, mccmy, mccmtheta, mccmphi;
  Double_t mcpx, mcpy, mcpz, mcp, mce, mcy, mctheta, mcphi;
  TTree *mctrktr = new TTree("mctrktree","mc track tree");
  mctrktr->Branch("pdg", &pdg, "pdg/I");

  mctrktr->Branch("mccmpx", &mccmpx, "mccmpx/D");
  mctrktr->Branch("mccmpy", &mccmpy, "mccmpy/D");
  mctrktr->Branch("mccmpz", &mccmpz, "mccmpz/D");
  mctrktr->Branch("mccmp", &mccmp, "mccmp/D");
  mctrktr->Branch("mccmy", &mccmy, "mccmy/D");
  mctrktr->Branch("mccmtheta", &mccmtheta, "mccmtheta/D");
  mctrktr->Branch("mccmphi", &mccmphi, "mccmphi/D");

  mctrktr->Branch("mcpx", &mcpx, "mcpx/D");
  mctrktr->Branch("mcpy", &mcpy, "mcpy/D");
  mctrktr->Branch("mcpz", &mcpz, "mcpz/D");
  mctrktr->Branch("mcp", &mcp, "mcp/D");
  mctrktr->Branch("mcy", &mcy, "mcy/D");
  mctrktr->Branch("mctheta", &mctheta, "mctheta/D");
  mctrktr->Branch("mcphi", &mcphi, "mcphi/D");

  mctrktr->Branch("ntrk",  &ntrk,  "ntrk/I");
  mctrktr->Branch("vtxx", &vtxx, "vtxx/D");
  mctrktr->Branch("vtxy", &vtxy, "vtxy/D");
  mctrktr->Branch("vtxz", &vtxz, "vtxz/D");
  mctrktr->Branch("aoq",  &aoq,  "aoq/D");
  mctrktr->Branch("zet",  &zet,  "zet/D");
  mctrktr->Branch("ggclose",  &ggclose,  "ggclose/I");

  Int_t ndf, good, nclus, nlclus, nrclus, neclus, nueclus, charge, kfcharge;
  Double_t px, py, pz, pzini, p, e, y, theta, phi, dedx, chi2;
  Double_t recopx, recopy, recopz, recop, recoe, recoy, recotheta, recophi, recocharge;
  Double_t vaposx, vaposy, vaposz;
  Double_t tgtpos, tgtx, tgty;
  Double_t pocam, pocax, pocay, pocaz, dist;
  Double_t recopocam, recopocax, recopocay, recopocaz, recodist;
  TTree *trktr = new TTree("trktree","track tree");
  trktr->Branch("ntrk", &ntrk, "ntrk/I");
  trktr->Branch("ngrtrk", &ngrtrk, "ngrtrk/I");
  trktr->Branch("nvtxtrk", &nvtxtrk, "nvtxtrk/I");
  trktr->Branch("vtxx", &vtxx, "vtxx/D");
  trktr->Branch("vtxy", &vtxy, "vtxy/D");
  trktr->Branch("vtxz", &vtxz, "vtxz/D");
  trktr->Branch("aoq",  &aoq,  "aoq/D");
  trktr->Branch("zet",  &zet,  "zet/D");
  trktr->Branch("ggclose",  &ggclose,  "ggclose/I");
  trktr->Branch("kfcharge", &kfcharge, "kfcharge/I");

  trktr->Branch("charge", &charge, "charge/I");
  trktr->Branch("px", &px, "px/D");
  trktr->Branch("py", &py, "py/D");
  trktr->Branch("pz", &pz, "pz/D");
  //  trktr->Branch("pzini", &pzini, "pzini/D");
  trktr->Branch("p", &p, "p/D");
  trktr->Branch("e", &e, "e/D");
  trktr->Branch("y", &y, "y/D");
  trktr->Branch("theta", &theta, "theta/D");
  trktr->Branch("phi", &phi, "phi/D");

  trktr->Branch("recocharge", &recocharge, "recocharge/I");
  trktr->Branch("recopx", &recopx, "recopx/D");
  trktr->Branch("recopy", &recopy, "recopy/D");
  trktr->Branch("recopz", &recopz, "recopz/D");
  //  trktr->Branch("pzini", &pzini, "pzini/D");
  trktr->Branch("recop", &recop, "recop/D");
  trktr->Branch("recoe", &recoe, "recoe/D");
  trktr->Branch("recoy", &recoy, "recoy/D");
  trktr->Branch("recotheta", &recotheta, "recotheta/D");
  trktr->Branch("recophi", &recophi, "recophi/D");

  trktr->Branch("vaposx", &vaposx, "vaposx/D");
  trktr->Branch("vaposy", &vaposy, "vaposy/D");
  trktr->Branch("vaposz", &vaposz, "vaposz/D");

  trktr->Branch("tgtpos", &tgtpos, "tgtpos/D");
  trktr->Branch("tgtx", &tgtx, "tgtx/D");
  trktr->Branch("tgty", &tgty, "tgty/D");
  trktr->Branch("pocam", &pocam, "pocam/D");
  trktr->Branch("pocax", &pocax, "pocax/D");
  trktr->Branch("pocay", &pocay, "pocay/D");
  trktr->Branch("pocaz", &pocaz, "pocaz/D");
  trktr->Branch("dist", &dist, "dist/D");
  trktr->Branch("recopocax", &recopocax, "recopocax/D");
  trktr->Branch("recopocay", &recopocay, "recopocay/D");
  trktr->Branch("recopocaz", &recopocaz, "recopocaz/D");
  trktr->Branch("recodist", &recodist, "recodist/D");
  trktr->Branch("dedx", &dedx, "dedx/D");
  trktr->Branch("chi2", &chi2, "chi2/D");
  trktr->Branch("ndf", &ndf, "ndf/I");
  trktr->Branch("neclus", &neclus, "neclus/I");
  trktr->Branch("nueclus", &nueclus, "nueclus/I");
  trktr->Branch("nclus", &nclus, "nclus/I");
  trktr->Branch("nlclus", &nlclus, "nlclus/I");
  trktr->Branch("nrclus", &nrclus, "nrclus/I");

  trktr->Branch("mccmpx", &mccmpx, "mccmpx/D");
  trktr->Branch("mccmpy", &mccmpy, "mccmpy/D");
  trktr->Branch("mccmpz", &mccmpz, "mccmpz/D");
  trktr->Branch("mccmp", &mccmp, "mccmp/D");
  trktr->Branch("mccmy", &mccmy, "mccmy/D");
  trktr->Branch("mccmtheta", &mccmtheta, "mccmtheta/D");
  trktr->Branch("mccmphi", &mccmphi, "mccmphi/D");
  trktr->Branch("mcpx", &mcpx, "mcpx/D");
  trktr->Branch("mcpy", &mcpy, "mcpy/D");
  trktr->Branch("mcpz", &mcpz, "mcpz/D");
  trktr->Branch("mcp", &mcp, "mcp/D");
  trktr->Branch("mcy", &mcy, "mcy/D");
  trktr->Branch("mctheta", &mctheta, "mctheta/D");
  trktr->Branch("mcphi", &mcphi, "mcphi/D");

  if(neventana>0) nEvents = neventana;
  //  nEvents = 10000;
  for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
      
    if(iEvent%1000 == 0) std::cout << "Event:" << iEvent << std::endl;
    fChain -> GetEntry(iEvent);
    //    mcChain -> GetEntry(iEvent);

    nvtx = vtxArray -> GetEntries();
    //ntrk = vtrkArray -> GetEntries();
    ngrtrk = eventHeader->GetNGoodRecoTrk();
    ntrk = eventHeader->GetNVATrk();
    //    nrtrk = rtrkArray -> GetEntries();
    nrtrk = eventHeader->GetNRecoTrk();
    nmctrk = mcArray -> GetEntries();
    if(isEmbed)
      if(nvtx!=1){
	std::cout <<"Event:"<< iEvent << " nvtx=" << nvtx << ", continue." << std::endl;
	continue;
      }
    if(nmctrk!=1){
      std::cout <<"Event:"<< iEvent << " nmctrk=" << nmctrk << ", continue." << std::endl;
      continue;
    }

    netrk = 0; nuetrk = 0; ngetrk = 0; ngeetrk = 0; nggeetrk = 0; nmggeetrk = 0;

    TVector3 vpos;
    vtxx = -9999; vtxy = -9999; vtxz = -9999; nvtxtrk = -9999; aoq = -9999; zet = -9999;
    aoq = eventHeader->GetAoQ();
    zet = eventHeader->GetZ();
    ggclose = eventHeader->IsGGCloseEvent();
    for (Int_t itr = 0; itr < nvtx && itr < 1; itr++) {
      STVertex *vtx = (STVertex *) vtxArray -> At(itr);
      vpos = vtx->GetPos();
      vtxx = vpos.X();
      vtxy = vpos.Y();
      vtxz = vpos.Z();
      nvtxtrk = vtx->GetNTracks();
    }

    Bool_t isGoodEmbedEvent = kFALSE;
    if(dstlist.Contains("embed132"))
      if(TMath::Abs(aoq-2.64)<0.01 && TMath::Abs(zet-49.9)<0.5) // Sn-132
	if(TMath::Abs(vtxx)<12 && TMath::Abs(vtxy+205)<10 && TMath::Abs(vtxz+15)<6) // Sn-132
	  isGoodEmbedEvent = kTRUE;
    if(dstlist.Contains("embed108"))
      if(abs(aoq-2.16)<0.01 && abs(zet-49.9)<0.5) // Sn-108
	if(abs(vtxx)<10 && abs(vtxy+206)<10 && abs(vtxz+14)<6) // Sn-108
	  isGoodEmbedEvent = kTRUE;

    for (Int_t itr = 0; itr < nmctrk; itr++) {
      STMCTrack *trk = (STMCTrack *) mcArray -> At(itr);
      pdg = trk->GetPdgCode();
      mcpx = trk->GetPx() * 1000; // GeV/c -> MeV/c
      mcpy = trk->GetPy() * 1000;
      mcpz = trk->GetPz() * 1000;
      mcp = sqrt(mcpx * mcpx + mcpy * mcpy + mcpz * mcpz);
      mce = sqrt(mass*mass + mcp*mcp);
      mcy = 0.5*TMath::Log((mce+mcpz)/(mce-mcpz));
      TVector3 mcmom = TVector3(mcpx,mcpy,mcpz);
      //      mctheta = atan2(sqrt(mcpx*mcpx + mcpy*mcpy),mcpz);
      mctheta = mcmom.Theta();
      mcphi = mcmom.Phi();

      TLorentzVector mccmmom;
      mccmmom.SetPxPyPzE(mcpx,mcpy,mcpz,mce);
      mccmmom.RotateY(-1*beamangle);

      mccmpx = mccmmom.Px();
      mccmpy = mccmmom.Py();
      mccmpz = mccmmom.Pz();
      mccmp = sqrt(mccmpx * mccmpx + mccmpy * mccmpy + mccmpz * mccmpz);
      mccme = sqrt(mass*mass + mccmp*mccmp);
      mccmy = mccmmom.Rapidity();
      mccmtheta = mccmmom.Theta();
      mccmphi = mccmmom.Phi();

      mctrktr->Fill();

    }

    for (Int_t itr = 0; itr < vtrkArray -> GetEntriesFast() ; itr++) {

      STRecoTrack *trk = (STRecoTrack *) vtrkArray -> At(itr);
      TVector3 mom = trk -> GetMomentum();
      Int_t helixid = trk->GetHelixID();
      //TVector3 momini = trk -> GetMomentumIni();
      if(mom.Mag() == 0) continue;
      px = mom.X(); py = mom.Y(); pz = mom.Z();
      //pzini = momini.Z();
      charge = trk -> GetCharge();
      kfcharge = trk -> GetGenfitCharge();
      //if(pzini<0) kfcharge *= -1;
      p = mom.Mag() * charge;
      e = sqrt(mass*mass + p*p);
      y = 0.5*TMath::Log((e+pz)/(e-pz));
      theta = mom.Theta();
      phi = mom.Phi();

      dedx = trk -> GetdEdxWithCut(0,0.7);                                      
      vaposx = trk -> GetPosVtxCluster().X();
      vaposy = trk -> GetPosVtxCluster().Y();
      vaposz = trk -> GetPosVtxCluster().Z();

      tgtpos = trk -> GetPosTargetPlane().Mag();
      tgtx = trk -> GetPosTargetPlane().X();
      tgty = trk -> GetPosTargetPlane().Y();

      TVector3 poca = trk -> GetPOCAVertex();
      pocam = poca.Mag();
      pocax = poca.X();
      pocay = poca.Y();
      pocaz = poca.Z();
      dist = sqrt((vtxx-pocax)*(vtxx-pocax)+(vtxy-pocay)*(vtxy-pocay)+(vtxz-pocaz)*(vtxz-pocaz));

      neclus = trk -> GetNumEmbedClusters();
      //      nueclus = trk -> GetNumUsedEmbedClusters();
      nclus = (trk -> GetClusterIDArray())->size()-1;
      nlclus = trk -> GetNumLayerClusters();
      nrclus = trk -> GetNumRowClusters();
      //nlclus = trk -> GetNumClusters();
      //nrclus = trk -> GetNumClusters();
      chi2 = trk -> GetChi2();
      ndf = trk -> GetNDF();

      recopx = -9999; recopy = -9999; recopz = -9999; recop = -9999;
      recocharge = -9999; recoe = -9999; recoy = -9999;
      recotheta = -9999; recophi = -9999;
      for (Int_t ritr = 0; ritr < rtrkArray -> GetEntriesFast(); ritr++) {
	STRecoTrack *recotrk = (STRecoTrack *) rtrkArray -> At(ritr);
	Int_t rhelixid = recotrk->GetHelixID();
	if(helixid == rhelixid){
	  TVector3 recomom = recotrk -> GetMomentum();
	  TVector3 recopoca = recotrk -> GetPOCAVertex();
	  recopx = recomom.X(); recopy = recomom.Y(); recopz = recomom.Z();
	  recocharge = recotrk -> GetCharge();
	  recop = recomom.Mag() * recocharge;
	  recoe = sqrt(mass*mass + recop*recop);
	  recoy = 0.5*TMath::Log((recoe+recopz)/(recoe-recopz));
	  recotheta = recomom.Theta();
	  recophi = recomom.Phi();

	  recopocax = recopoca.X();
	  recopocay = recopoca.Y();
	  recopocaz = recopoca.Z();
	  recodist = sqrt((vtxx-recopocax)*(vtxx-recopocax)+(vtxy-recopocay)*(vtxy-recopocay)+(vtxz-recopocaz)*(vtxz-recopocaz));

	}
      }

      if(isEmbed){
	if(neclus>0){
	  netrk ++;
	  if(((Double_t)neclus)/((Double_t)(nlclus+nrclus))>0.5){
	    ngetrk ++;
	    if(isGoodEmbedEvent){
	      ngeetrk ++;
	      if(sqrt(tgtx*tgtx+(tgty+232)*(tgty+232))<50&&ndf>20&&TMath::Abs(p)>0)
		if(atan(py/pz)/3.14*180>-15&&atan(py/pz)/3.14*180<10&&atan(-px/pz)/3.14*180<0&&atan(-px/pz)/3.14*180>-30){ // cut for fiducial area
		  nggeetrk ++;
		  if(TMath::Abs(y-0.35)<0.05)
		    nmggeetrk ++;
	      }
	    }
	  }
	  trktr->Fill();
	}
      }
      else{
	trktr->Fill();
      }

    }// for (Int_t itr = 0; itr < ntrk; itr++)

    evetr->Fill();

  }

  fout->Write();
  fout->Close();
  
}
