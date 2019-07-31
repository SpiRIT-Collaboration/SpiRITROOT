void Produce_Theta_TargetPos()
{
  bool IsOption_CheckCorrection = 1; // after you get the BDC correction file, you can use this option to check does the correction files can correct the postion of BDC.
  
  string BDCCorrection_FileName = "./f1_BDCCorrection_Theta_TargetPos.root";
  
  TChain* tree = new TChain("cbmsim");
  tree->AddFile("../data/run2894_s0.40kEvts.noBDCShift.reco.develop.1820.549ad58.root");
  
  TClonesArray *VertexArray = nullptr;
  TClonesArray *RecoTrackArray = nullptr;
  TClonesArray *VATrackArray = nullptr;
  
  tree -> SetBranchAddress("STVertex", &VertexArray);
  tree -> SetBranchAddress("STRecoTrack", &RecoTrackArray);
  tree -> SetBranchAddress("VATracks", &VATrackArray);
  
  Int_t nEvents = tree -> GetEntries();
  cout<<"EvtNum in the tree: "<<nEvents<<endl;
  
  TH2D* h2_Theta_Phi = new TH2D("h2_Theta_Phi",";Theta(Deg.);Phi(Deg.)",90,0,90,360,0,360);
  TH1D* h1_Track_TargetPosZ = new TH1D("h1_Track_TargetPosZ",";PosZ(mm);Count",40,-30,10);
  
  const int PhiNum = 36; // here, the phi is divided into 36, each part cover 10 deg.
  int PhiRange[PhiNum][2];
  for(int i=0;i<PhiNum;i++)
  {
    PhiRange[i][0] = i*10;
    PhiRange[i][1] = i*10+10;
  }
  
  TH2D* h2_Theta_TargetPosX[PhiNum] = {0};
  TH2D* h2_Theta_TargetPosY[PhiNum] = {0};
  
  char NameTem[200];
  for(int i=0;i<PhiNum;i++)
  {
    sprintf(NameTem,"h2_Theta_TargetPosX_Phi_%d_%d",PhiRange[i][0],PhiRange[i][1]);
    h2_Theta_TargetPosX[i] = new TH2D(NameTem,";Theta(Deg.);Target PosX(mm)",90,0,90,400,-40,40);
    
    sprintf(NameTem,"h2_Theta_TargetPosY_Phi_%d_%d",PhiRange[i][0],PhiRange[i][1]);
    h2_Theta_TargetPosY[i] = new TH2D(NameTem,";Theta(Deg.);Target PosY(mm)",90,0,90,400,-40,40);
  }
  
  //ST_VertexShift is added.
  ST_VertexShift* Vertex_Shifter = 0; 
  if(IsOption_CheckCorrection==1)
  {
    Vertex_Shifter = new ST_VertexShift();
    Vertex_Shifter->Load_BDC_Correction("f1_BDCCorrection_Theta_TargetPos.root");
  }
  
  int EffectEvtNum = 0;
  for(int iEvt=0;iEvt<nEvents*1;iEvt++)
  {
    if(iEvt%1000==0) { cout<<"iEvt: "<<iEvt<<endl; }
    tree -> GetEntry(iEvt);
    
    int VertexNum = VertexArray->GetEntries();
    int RecoTrackNum = RecoTrackArray->GetEntries();
    int VATrackNum = VATrackArray->GetEntries();

    //only analyze the vertex on the target.
    if(VertexNum!=1) { continue; }
    STVertex* aVertex = (STVertex*) VertexArray -> At(0);
    TVector3 Pos_Vertex = aVertex->GetPos();
    if( !(Pos_Vertex.Z()> -13.2 -1.7*3 && Pos_Vertex.Z()< -13.2 +1.7*3) ) { continue; }
    EffectEvtNum++;
    
    //for checking the RecoTrack
    for(int iRecoTrack=0;iRecoTrack<RecoTrackNum;iRecoTrack++)
    {
      STRecoTrack* aRecoTrack = (STRecoTrack*) RecoTrackArray -> At(iRecoTrack);
      TVector3 RecoMom = aRecoTrack->GetMomentumTargetPlane();
      TVector3 PosOnTarget = aRecoTrack->GetPosTargetPlane();
      
      if(RecoMom.Mag()==0) { continue; }
      int RowClusterNum = aRecoTrack->GetNumRowClusters();
      int LayerClusterNum = aRecoTrack->GetNumLayerClusters();
      
      //here, I want to get a clear bias of BDC, so I use ClusterNum>30 to choose some good track.
      if(RowClusterNum+LayerClusterNum<30) { continue; } 
      
      double Charge = aRecoTrack->GetCharge();
      double dedx_Mom = aRecoTrack->GetdEdxWithCut(0.1,0.7);
      if(Charge<0) { continue; } 
      
      double Phi = RecoMom.Phi()*TMath::RadToDeg();
      double Theta = RecoMom.Theta()*TMath::RadToDeg();
      if(Phi<0) { Phi = Phi+360; }
      
      h2_Theta_Phi->Fill(Theta,Phi);
      
      double ShiftX = 0; double ShiftY = 0;
      if(IsOption_CheckCorrection==1)
      { Vertex_Shifter->GetShiftValue_Smooth(Theta,Phi,&ShiftX,&ShiftY); }
      
      int PhiIndex = -1;
      for(int iPhi=0;iPhi<PhiNum;iPhi++)
      { if(Phi>PhiRange[iPhi][0] && Phi<=PhiRange[iPhi][1]) { PhiIndex = iPhi; break;} }
      
      h1_Track_TargetPosZ->Fill(PosOnTarget.Z());
      
      if(PhiIndex!=-1)
      {
        h2_Theta_TargetPosX[PhiIndex]->Fill(Theta,(PosOnTarget-Pos_Vertex).X()-ShiftX);
        h2_Theta_TargetPosY[PhiIndex]->Fill(Theta,(PosOnTarget-Pos_Vertex).Y()-ShiftY);
      } 
    }
  }
  cout<<"The target event number : "<<EffectEvtNum<<endl;
  
  //the below is for drawing;
  TCanvas* c1_Theta_Phi = new TCanvas("c1_Theta_Phi","c1_Theta_Phi",1);
  h2_Theta_Phi->Draw("colz");
  
  TCanvas* c1_Theta_TargetPosX = new TCanvas("c1_Theta_TargetPosX","c1_Theta_TargetPosX",1);
  c1_Theta_TargetPosX->Divide(6,6);
  for(int i=0;i<PhiNum;i++)
  {
    c1_Theta_TargetPosX->cd(i+1);
    h2_Theta_TargetPosX[i]->Draw("colz");
  }
  
  TCanvas* c1_Theta_TargetPosY = new TCanvas("c1_Theta_TargetPosY","c1_Theta_TargetPosY",1);
  c1_Theta_TargetPosY->Divide(6,6);
  for(int i=0;i<PhiNum;i++)
  {
    c1_Theta_TargetPosY->cd(i+1);
    h2_Theta_TargetPosY[i]->Draw("colz");
  }
  
  // the below is for writing the histogram into the file, which is the input of ST_VertexShift.
  if(IsOption_CheckCorrection==0)
  {
    TFile* f1_Theta_TargetPos = new TFile(BDCCorrection_FileName.c_str(),"recreate");
    f1_Theta_TargetPos->cd();
    h2_Theta_Phi->Write("",TObject::kOverwrite);  
    for(int i=0;i<PhiNum;i++)
    {
      h2_Theta_TargetPosX[i]->Write("",TObject::kOverwrite);
      h2_Theta_TargetPosY[i]->Write("",TObject::kOverwrite);
    }
    f1_Theta_TargetPos->Close();
    ST_VertexShift* VertexShifter = new ST_VertexShift();
    VertexShifter->Cal_BDC_Correction(BDCCorrection_FileName.c_str());
  }
}
