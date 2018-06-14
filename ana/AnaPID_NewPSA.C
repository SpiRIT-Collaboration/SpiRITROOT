void AnaPID_NewPSA()
{
  auto tree = new TChain("cbmsim");
  TString filename = "./data/_run2894_s0_5000Evts_PSA_PeakFinding_2.reco.root";
//  TString filename = "./data/_run2894_s0_50Evts_PSA_PeakFinding_2.reco.root";
  tree -> AddFile(filename);
  TClonesArray *RecoTrackArray = nullptr;
  TClonesArray *HelixArray = nullptr;
  TClonesArray *VertexArray = nullptr;
  TClonesArray *HitArray = nullptr;
  tree -> SetBranchAddress("STHelixTrack", &HelixArray);
  tree -> SetBranchAddress("STRecoTrack", &RecoTrackArray);
  tree -> SetBranchAddress("STVertex", &VertexArray);
  tree -> SetBranchAddress("STHit", &HitArray);
  
  auto histPID = new TH2D("histPID",";p/Z (MeV/Z); dEdx (ADC/mm)",300,-500,2500,400,0,800);
  auto histEmitAngle = new TH2D("histEmitAngle",";#Theta (Deg.); #Phi (Deg.)",90,0,90,180,-180,180);
  TH1D* h1_HitNum_PerTrack = new TH1D("h1_HitNum_PerTrack",";HitNum_PerTrack;count",200,0,200);
  
  TCanvas* c1_Track_EvtDis = new TCanvas("c1_Track_EvtDis","c1_Track_EvtDis",1200,800);
  
  TH2D* h2_Track_TopPlane = new TH2D("h2_Track_TopPlane",";LayerIndex(Z axis);RowIndex(X axis)",112,0,112,108,0,108);
  TH2D* h2_Track_YZSide = new TH2D("h2_Track_YZSide",";LayerIndex(Z axis);Tb(Y axis)",112,0,112,270,0,270);
  
  TH2D* h2_ClusterNum_Exp_Theo = new TH2D("h2_ClusterNum_Exp_Theo",";Theo. ClusterNum ;Exp. ClusterNum",200,0,200,200,0,200);
  
  STPickTrackCut* aTrackCut = new STPickTrackCut();
  aTrackCut->Set_Track_DistanceToVertex(5);
  
  //the below is for 
  ST_ClusterNum_DB* db = new ST_ClusterNum_DB();
  db->Initial_Config("Momentum.config");
  db->ReadDB("f1_DB_ClusterNum.root");
  double Momentum_Range_Plus[2] = {50,3000};
  double Momentum_Range_Minus[2] = {50,1000};
  db->Set_MomentumRange_Plus(Momentum_Range_Plus);
  db->Set_MomentumRange_Minus(Momentum_Range_Minus);
  
  Int_t nEvents = tree -> GetEntries();
  cout<<"EvtNum: "<<nEvents<<endl;
//  nEvents = 50;
  
  for(int iEvent=0;iEvent<nEvents;iEvent++)
  {
    if(iEvent%100==0) { cout<<"Evt: "<< iEvent<<endl; }
    tree -> GetEntry(iEvent);
    if(VertexArray->GetEntries()==0) { continue; }
    STVertex* aVertex = (STVertex*) VertexArray -> At(0);
    if(aTrackCut->IsVertexInTarget(aVertex)==0) { continue; }
    int numRecoTrack = RecoTrackArray -> GetEntries();
    char first;
    
    for(int iTrack = 0;iTrack<numRecoTrack;iTrack++)
    {
      STRecoTrack* aTrack = (STRecoTrack*) RecoTrackArray -> At(iTrack);
      if(aTrackCut->IsTrackInVertex(aTrack,aVertex)==0) { continue; }
      vector<Int_t>* fHitClusterIDArray = aTrack->GetClusterIDArray();
      int ClusterNum = (*fHitClusterIDArray).size();
      int ClusterNum_Theo = db->GetClusterNum(aTrack->GetCharge(),aTrack->GetMomentum());
      if(ClusterNum_Theo>1) { h2_ClusterNum_Exp_Theo->Fill(ClusterNum_Theo,ClusterNum); }
//      if(ClusterNum<=20) { continue; }
      if(ClusterNum<=20 || ( aTrack->GetCharge()>0 && ((double)ClusterNum)/((double) ClusterNum_Theo)<0.7 ) ) { continue; }
      
      int HelixID = aTrack->GetHelixID();
      STHelixTrack* aHelixTrack = (STHelixTrack*) HelixArray->At(HelixID);
      double fRMSW = aHelixTrack->GetRMSW();
      double fRMSH = aHelixTrack->GetRMSH();
//      if(fRMSW>5) { continue; }
//      if( aTrackCut->IsInPionMinusCut(aTrack)==1 || aTrackCut->IsInPionPlusCut(aTrack)==1 )
//      if( aTrackCut->IsBetween_Pion_Proton(aTrack)==1 )
//      if(iEvent==29)
      {
        double Charge = aTrack->GetCharge();
        auto p =  aTrack -> GetMomentum().Mag();
        auto dedx = aTrack -> GetdEdxWithCut(0,0.7);
        double dist = (aTrack->GetPOCAVertex()-aVertex->GetPos()).Mag();
        histPID->Fill(p/Charge,dedx);
        double MomentumTheta = aTrack->GetMomentumTargetPlane().Theta()*180.0/Pi();
        double MomentumPhi = aTrack->GetMomentumTargetPlane().Phi()*180.0/Pi();
        histEmitAngle->Fill(MomentumTheta,MomentumPhi);
/*
        int HitNum = HitArray->GetEntries();
        int HitNumPreTrack = 0;
        h2_Track_TopPlane->Reset();
        h2_Track_YZSide->Reset();
        for(int iHit=0;iHit<HitNum;iHit++)
        {
          STHit* aHit = (STHit*) HitArray->At(iHit);
          if(aHit->GetTrackID()==iTrack && aHit->IsClustered()==1) 
          {
            int LayerIndex = aHit->GetLayer();
            int RowIndex = aHit->GetRow();
            double fCharge = aHit->GetCharge();
            double fTb = aHit->GetTb();
            h2_Track_TopPlane->Fill(LayerIndex,RowIndex,fCharge);
            h2_Track_YZSide->Fill(LayerIndex,fTb,fCharge);
            HitNumPreTrack++; 
          }
        }
*/
 /*       
        cout<<" fRMSW: "<<fRMSW<<" fRMSH: "<<fRMSH<<endl;
        cout<<" Momentum:  "<< p <<" distance with vertex: "<< dist <<endl;
        h1_HitNum_PerTrack->Fill(HitNumPreTrack);
        c1_Track_EvtDis->Clear();
        c1_Track_EvtDis->Divide(1,2);
        c1_Track_EvtDis->cd(1);
        h2_Track_TopPlane->Draw("colz");
        c1_Track_EvtDis->cd(2);
        h2_Track_YZSide->Draw("colz");
        c1_Track_EvtDis->Update();
        first = getchar();
        if(first=='q') { break; }
*/
      }
    }
/*
    if(first=='q') 
    {
      first=getchar();
      if(first=='q') break;
    }
*/
  }

  TCanvas* c1_PID = new TCanvas("c1_PID","c1_PID",1);
  histPID->Draw("colz");
  aTrackCut->GetPionPlusCut()->Draw("same");
  aTrackCut->GetPionMinusCut()->Draw("same");
  
  TCanvas* c1_ThetaPhi = new TCanvas("c1_ThetaPhi","c1_ThetaPhi",1);
  histEmitAngle->Draw("colz");
  
  TCanvas* c1_HitNum_PerTrack = new TCanvas("c1_HitNum_PerTrack","c1_HitNum_PerTrack",1);
  c1_HitNum_PerTrack->cd();
  h2_ClusterNum_Exp_Theo->Draw("colz");
//  h1_HitNum_PerTrack->Draw();
}
