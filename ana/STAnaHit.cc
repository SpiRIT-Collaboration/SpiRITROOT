#include "STAnaHit.hh"

ClassImp(STAnaHit)

STAnaHit::STAnaHit()
{
  IsAna_HitDensity = 0;
  
  fHitArray = 0;
  fHitClusterArray = 0;
  fHelixTrackArray = 0;
  fRecoTrackArray = 0;
  
  h1_HitNum_1Pad = 0;
  h1_UsedHitNum_1Pad = 0;
  h1_Hit_UsedRatio_Total = 0;
  c1_HitNum_1Pad = 0;
  
  h1_Hit_UsedRatio_1Pad = 0;
  h1_Hit_UsedRatio_HitDensity_30Tb = 0;
  h1_Hit_UsedRatio_HitDensity_20Tb = 0;
  h1_Hit_UsedRatio_HitDensity_10Tb = 0;
  c1_Hit_UsedRatio = 0;
  outfile.open("UsedHit_LowEff.txt");
  
  aTrackCut = new STPickTrackCut();
  
  h2_MultiHit_PadPlane = 0;
  h2_MultiHit_SideView = 0;
  c1_MultiHit_Dis = 0;
  AnaEvtNum = 0;
  
  h1_TrackResErr = 0;
  h2_TrackResErr_HitDensity = 0;
  c1_TrackResErr = 0;
  //the saturated pulse
  SaturatedPadNum = 0;
  h2_Saturated_FollowHit = 0;
  c1_Saturated_FollowHit = 0;
}

STAnaHit::~STAnaHit()
{ }

void STAnaHit::Initial(string tag)
{
  AnaEvtNum = 0;
  
  char NameTem[200];
  if(h1_HitNum_1Pad==0)
  { 
    sprintf(NameTem,"h1_HitNum_1Pad_%s",tag.c_str());
    h1_HitNum_1Pad = new TH1D(NameTem,";HitNum per 1 Pad;Count",50,0,50); 
  }
  else { h1_HitNum_1Pad->Reset(); }
  
  if(h1_UsedHitNum_1Pad==0)
  { 
    sprintf(NameTem,"h1_UsedHitNum_1Pad_%s",tag.c_str());
    h1_UsedHitNum_1Pad = new TH1D(NameTem,";Used HitNum per 1 Pad;Count",50,0,50); 
  }
  else { h1_UsedHitNum_1Pad->Reset(); }
  
  if(h1_Hit_UsedRatio_Total==0)
  { 
    sprintf(NameTem,"h1_Hit_UsedRatio_Total_%s",tag.c_str());
    h1_Hit_UsedRatio_Total = new TH1D(NameTem,";Hit Used Ratio;Count",100,0,1); 
  }
  else { h1_Hit_UsedRatio_Total->Reset(); }
  
  if(h1_Hit_UsedRatio_1Pad==0)
  {
    sprintf(NameTem,"h1_Hit_UsedRatio_1Pad_%s",tag.c_str());
    h1_Hit_UsedRatio_1Pad = new TH1D(NameTem,";HitNum/1 pad; UsedRatio",50,0,50);
  }
  // the below is for studying the hit density 
  if(h1_Hit_UsedRatio_HitDensity_30Tb==0)
  {
    sprintf(NameTem,"h1_Hit_UsedRatio_HitDensity_30Tb_%s",tag.c_str());
    h1_Hit_UsedRatio_HitDensity_30Tb = new TH1D(NameTem,";HitNum/30 Tb;UsedRatio",50,0,50);
  }
  
  if(h1_Hit_UsedRatio_HitDensity_20Tb==0)
  {
    sprintf(NameTem,"h1_Hit_UsedRatio_HitDensity_20Tb_%s",tag.c_str());
    h1_Hit_UsedRatio_HitDensity_20Tb = new TH1D(NameTem,";HitNum/20 Tb;UsedRatio",50,0,50);
  }
  
  if(h1_Hit_UsedRatio_HitDensity_10Tb==0)
  {
    sprintf(NameTem,"h1_Hit_UsedRatio_HitDensity_10Tb_%s",tag.c_str());
    h1_Hit_UsedRatio_HitDensity_10Tb = new TH1D(NameTem,";HitNum/10 Tb;UsedRatio",50,0,50);
  }
  
  for(int i=0;i<50;i++) 
  {
    TotalHitNum_1Pad[i] = 0; TotalUsedHitNum_1Pad[i] = 0;
    TotalHitNum_30Tb[i] = 0; TotalUsedHitNum_30Tb[i] = 0;
    TotalHitNum_20Tb[i] = 0; TotalUsedHitNum_20Tb[i] = 0;
    TotalHitNum_10Tb[i] = 0; TotalUsedHitNum_10Tb[i] = 0;
  }
  
  //the below is for studying the multi-hit distribution in the top plane.
  if( h2_MultiHit_PadPlane==0 )
  {
    sprintf(NameTem,"h2_MultiHit_PadPlane_%s",tag.c_str());
    h2_MultiHit_PadPlane = new TH2D(NameTem,";Z(Layer Num);X(Row Num)",112,0,112,108,0,108);
  }
  if( h2_MultiHit_SideView==0 )
  {
    sprintf(NameTem,"h2_MultiHit_SideView_%s",tag.c_str());
    h2_MultiHit_SideView = new TH2D(NameTem,";Z(Layer Num);Tb(Y axis)",112,0,112,270,-270,0);
  }
  
  if(h1_TrackResErr == 0)
  {
    sprintf(NameTem,"h1_TrackResErr_%s",tag.c_str());
    h1_TrackResErr = new TH1D(NameTem,";Distance to Track; count",1000,0,100);
  }
  if( h2_TrackResErr_HitDensity==0 )
  {
    sprintf(NameTem,"h2_TrackResErr_HitDensity_%s",tag.c_str());
    h2_TrackResErr_HitDensity = new TH2D(NameTem,";Distance to Track/mm; HitDensity",100,0,100,200,0,20);
  }
  
  //for the saturated pulse
  if(h2_Saturated_FollowHit==0)
  {
    sprintf(NameTem,"h2_Saturated_FollowHit_%s",tag.c_str());
    h2_Saturated_FollowHit = new TH2D(NameTem,";Time Bucket; Count",270,0,270,350,0,3500);
  }
}

void STAnaHit::Ana_UsedHit(int EvtID)
{
  if(fHitArray==0) { cout<<"***ERROR: fHitArray=0"<<endl; return; }
  int HitNum_1Pad[112][108] = {{0}}; //hitnum int the pad
  int HitIndex_1Pad[112][108][30] = {{{-1}}}; //the hit index of a pad.
  double HitTb_1Pad[112][108][30] = {{{-1}}}; //the hit index of a pad.
  double HitCharge_1Pad[112][108][30] = {{{-1}}}; //the hit charge of a pad.
  double HitNum = fHitArray->GetEntries();
  double UsedHitNum_Total = 0;
  
  for(int iHit=0;iHit<HitNum;iHit++)
  {
    STHit* aHit = (STHit*) fHitArray->At(iHit);
    //the below is for making a map, or it will be very difficult to compare the hit calculation between different PSA method.
    int RowIndex = aHit->GetRow();
    int LayerIndex = aHit->GetLayer();
    double fTb = aHit->GetTb();
    double fCharge = aHit->GetCharge();
    int HitNum_tem = HitNum_1Pad[LayerIndex][RowIndex];
    if(HitNum_tem>50) { continue; }
    
    HitIndex_1Pad[LayerIndex][RowIndex][HitNum_tem] = iHit;
    HitTb_1Pad[LayerIndex][RowIndex][HitNum_tem] = fTb;
    HitCharge_1Pad[LayerIndex][RowIndex][HitNum_tem] = fCharge;
    
    HitNum_1Pad[LayerIndex][RowIndex] = HitNum_1Pad[LayerIndex][RowIndex]+1;
    int TrackID = aHit->GetTrackID();
    int IsClustered = aHit->IsClustered();
    if(TrackID!=-1 && IsClustered==1) { UsedHitNum_Total++; }
  }
  
  //the below is for sorting the hit with fTb
  for(int iRow=MinRowIndex;iRow<MaxRowIndex;iRow++)
  {
    for(int iLayer=MinLayerIndex;iLayer<MaxLayerIndex;iLayer++)
    {
      //here the hits have been sortted by the fTb
      int HitNum_tem = HitNum_1Pad[iLayer][iRow];
      for(int i=0;i<HitNum_tem-1;i++)
      {
        for(int j=i;j<HitNum_tem-1;j++)
        {
          if(HitTb_1Pad[iLayer][iRow][j]>HitTb_1Pad[iLayer][iRow][j+1])
          {
            //for the hit Tb
            double Tem = HitTb_1Pad[iLayer][iRow][j];
            HitTb_1Pad[iLayer][iRow][j] = HitTb_1Pad[iLayer][iRow][j+1];
            HitTb_1Pad[iLayer][iRow][j+1] = Tem;
            //for the hit Charge
            Tem = HitCharge_1Pad[iLayer][iRow][j];
            HitCharge_1Pad[iLayer][iRow][j] = HitCharge_1Pad[iLayer][iRow][j+1];
            HitCharge_1Pad[iLayer][iRow][j+1] = Tem;
            // for the hit index
            Tem = HitIndex_1Pad[iLayer][iRow][j];
            HitIndex_1Pad[iLayer][iRow][j] = HitIndex_1Pad[iLayer][iRow][j+1];
            HitIndex_1Pad[iLayer][iRow][j+1] = Tem;
          }
        }
      }
/*
      //here I would like to study the saturated pulse.
      double ChargeTotal = 0;
      if(HitNum_tem>2) 
      {
        cout<<"Row: "<<iRow<<"  Layer: "<<iLayer<<"  --->  ";
      }
      for(int i=0;i<HitNum_tem;i++)
      {
        cout<<HitTb_1Pad[iLayer][iRow][i]<<"  "<<HitCharge_1Pad[iLayer][iRow][i]<<" *** ";

        ChargeTotal += HitCharge_1Pad[iLayer][iRow][i];
        if(ChargeTotal>3500 && i<HitNum_tem-1)
        {
          SaturatedPadNum++;
          for(int j=i;j<HitNum_tem;j++)
          { h2_Saturated_FollowHit->Fill(HitTb_1Pad[iLayer][iRow][j+1]-HitTb_1Pad[iLayer][iRow][j],HitCharge_1Pad[iLayer][iRow][j+1]); }
          break;
        }

      }
      if(HitNum_tem>0) { cout<<endl; }
*/      
    }
  }
  
  //The below is for studying the hit density.
  if(UsedHitNum_Total/HitNum<0.4 && IsAna_HitDensity==1) { return; }
  AnaEvtNum++;
  for(int iRow=MinRowIndex;iRow<MaxRowIndex;iRow++)
  {
    for(int iLayer=MinLayerIndex;iLayer<MaxLayerIndex;iLayer++)
    {
      h1_HitNum_1Pad->Fill(HitNum_1Pad[iLayer][iRow]);
      double HitNumTem = 0;
      double HitNum_30Tb[50] = {0};
      double UsedHitNum_30Tb[50] = {0};
      double HitNum_20Tb[50] = {0};
      double UsedHitNum_20Tb[50] = {0};
      double HitNum_10Tb[50] = {0};
      double UsedHitNum_10Tb[50] = {0};
//      h2_MultiHit_PadPlane->Fill(iLayer,iRow,HitNum_1Pad[iLayer][iRow]);
      
      for(int iHit=0;iHit<HitNum_1Pad[iLayer][iRow];iHit++)
      {
        STHit* aHit = (STHit*) fHitArray->At(HitIndex_1Pad[iLayer][iRow][iHit]);
        int TrackID = aHit->GetTrackID();
        int IsClustered = aHit->IsClustered();
        double fCharge = aHit->GetCharge();
        double fTb = aHit->GetTb();
        HitNum_30Tb[(int)fTb/30] +=1;
        HitNum_20Tb[(int)fTb/20] +=1;
        HitNum_10Tb[(int)fTb/10] +=1;
        if(TrackID!=-1 && IsClustered==1) 
        {
          UsedHitNum_30Tb[(int)fTb/30] +=1;
          UsedHitNum_20Tb[(int)fTb/20] +=1;
          UsedHitNum_10Tb[(int)fTb/10] +=1;
          h2_MultiHit_PadPlane->Fill(iLayer,iRow,1);
          if(iRow>40 && iRow<70) { h2_MultiHit_SideView->Fill(iLayer,-fTb); }
          HitNumTem++; 
        }
//        if(iRow>40 && iRow<70) { h2_MultiHit_SideView->Fill(iLayer,-fTb); }
      }
      if(HitNumTem!=0) 
      {
        h1_UsedHitNum_1Pad->Fill(HitNumTem);
      }
      if(HitNum_1Pad[iLayer][iRow]>0)
      {
        // study the density of 1 pad
        TotalUsedHitNum_1Pad[HitNum_1Pad[iLayer][iRow]] += HitNumTem;
        TotalHitNum_1Pad[HitNum_1Pad[iLayer][iRow]] += 1;
        // study the density of 30Tb, 20Tb, 10Tb
        for(int iTb=0;iTb<50;iTb++)
        {
          if(HitNum_30Tb[iTb]>0) { TotalHitNum_30Tb[(int)HitNum_30Tb[iTb]] +=1; }
          if(UsedHitNum_30Tb[iTb]>0) { TotalUsedHitNum_30Tb[(int)UsedHitNum_30Tb[iTb]] +=1; }
          
          if(HitNum_20Tb[iTb]>0) { TotalHitNum_20Tb[(int)HitNum_20Tb[iTb]] +=1; }
          if(UsedHitNum_20Tb[iTb]>0) { TotalUsedHitNum_20Tb[(int)UsedHitNum_20Tb[iTb]] +=1; }
          
          if(HitNum_10Tb[iTb]>0) { TotalHitNum_10Tb[(int)HitNum_10Tb[iTb]] +=1; }
          if(UsedHitNum_10Tb[iTb]>0) { TotalUsedHitNum_10Tb[(int)UsedHitNum_10Tb[iTb]] +=1; }
        }
      }
    }
  }
  h1_Hit_UsedRatio_Total->Fill(UsedHitNum_Total/HitNum);

//  if(UsedHitNum_Total/HitNum<0.5) { cout<<"EvtID:"<<EvtID<<endl; outfile<<EvtID<<endl;}
  //the below is for analyzing the hit density and residual error of track
  int VertexNum = fVertexArray->GetEntries();
  if(VertexNum!=1) { return; }
  int TrackNum = fRecoTrackArray->GetEntries();
  STVertex* aVertex = (STVertex*) fVertexArray->At(0);
  for(int iTrack=0;iTrack<TrackNum;iTrack++)
  {
    STRecoTrack* aTrack = (STRecoTrack*) fRecoTrackArray->At(iTrack);
    if(aTrackCut->IsTrackInVertex(aTrack,aVertex)==0) { continue; }
    vector<Int_t> * ClusterIDArray = aTrack->GetClusterIDArray();
    int ClusterNum = (*ClusterIDArray).size();
    if(ClusterNum<30) { continue; }
    int HelixID = aTrack->GetHelixID();
    STHelixTrack* aHelixTrack = (STHelixTrack*) fHelixTrackArray->At(HelixID);
    
    for(int iCluster=0;iCluster<ClusterNum;iCluster++)
    {
      int ClusterID = (*ClusterIDArray)[iCluster];
      STHitCluster* aCluster = (STHitCluster*) fHitClusterArray->At(ClusterID);
      TVector3 PosOnTrack = aCluster->GetPOCA();
      TVector3 PosOfCluster = aCluster->GetPosition();
      double ResErr_tem = (PosOnTrack-PosOfCluster).Mag();
      h1_TrackResErr->Fill(ResErr_tem);
/*      
      vector<Int_t> *HitIDArray = aCluster->GetHitIDs();
      int HitNum_inCluster = (*HitIDArray).size();
      int HitNum_Arround_Total = 0;
      for(int iHit=0;iHit<HitNum_inCluster;iHit++)
      {
        int HitID_tem = (*HitIDArray)[iHit];
        STHit* aHit = (STHit*) fHitArray->At(HitID_tem);
        int RowIndex = aHit->GetRow();
        int LayerIndex = aHit->GetLayer();
        double fTb = aHit->GetTb();
        int HitNum_samePad = HitNum_1Pad[LayerIndex][RowIndex];
        if(HitNum_samePad<2) { continue; }
        
        double DeltaTb_Nearest = 500;
        for(int iHit_samePad=0;iHit_samePad<HitNum_samePad;iHit_samePad++)
        {
          int HitIndex_Arround = HitIndex_1Pad[LayerIndex][RowIndex][iHit_samePad];
          if(HitIndex_Arround==HitID_tem) { continue; }
          STHit* aHit_Arround = (STHit*) fHitArray->At(HitIndex_Arround);
          double fTb_Arround = aHit_Arround->GetTb();
          if(fTb_Arround-fTb<0 && fTb_Arround-fTb<DeltaTb_Nearest) { DeltaTb_Nearest = fTb_Arround-fTb; }
        }
//        if(DeltaTb_Nearest!=500) {  }
      }
//      h2_TrackResErr_HitDensity->Fill(ResErr_tem,((double)HitNum_Arround_Total)/((double)HitNum_inCluster));
*/
    }
  }
}

void STAnaHit::Draw()
{
  //the below is for drawing the distribution of hit num.
  if(c1_HitNum_1Pad!=0) { c1_HitNum_1Pad->Clear(); }
  else{ c1_HitNum_1Pad = new TCanvas("c1_HitNum_1Pad","c1_HitNum_1Pad",1); }
  c1_HitNum_1Pad->Divide(1,2);
  c1_HitNum_1Pad->cd(1)->SetLogy(1);
  h1_HitNum_1Pad->Draw("hist"); h1_HitNum_1Pad->SetLineColor(2); h1_HitNum_1Pad->SetLineWidth(2); 
  h1_UsedHitNum_1Pad->Draw("samehist"); h1_UsedHitNum_1Pad->SetLineWidth(2);
  c1_HitNum_1Pad->cd(2); h1_Hit_UsedRatio_Total->Draw(); h1_Hit_UsedRatio_Total->SetLineWidth(2);
  c1_HitNum_1Pad->Update();

  //the below is for drawing the distribution of hit density.
  for(int i=1;i<50;i++)
  {
    if(TotalHitNum_1Pad[i]!=0)
    { h1_Hit_UsedRatio_1Pad->Fill(i,TotalUsedHitNum_1Pad[i]/(i*TotalHitNum_1Pad[i])); }
    if( TotalHitNum_30Tb[i]!=0 )
    { h1_Hit_UsedRatio_HitDensity_30Tb->Fill(i,TotalUsedHitNum_30Tb[i]/(i*TotalHitNum_30Tb[i])); }
    if( TotalHitNum_20Tb[i]!=0 )
    { h1_Hit_UsedRatio_HitDensity_20Tb->Fill(i,TotalUsedHitNum_20Tb[i]/(i*TotalHitNum_20Tb[i])); }
    if( TotalHitNum_10Tb[i]!=0 )
    { h1_Hit_UsedRatio_HitDensity_10Tb->Fill(i,TotalUsedHitNum_10Tb[i]/(i*TotalHitNum_10Tb[i])); }
  }
  
  if(IsAna_HitDensity==1)
  {
    if(c1_Hit_UsedRatio!=0) { c1_Hit_UsedRatio->Clear(); }
    else{ c1_Hit_UsedRatio = new TCanvas("c1_Hit_UsedRatio","c1_Hit_UsedRatio",1); }
    c1_Hit_UsedRatio->Divide(1,3);
    c1_Hit_UsedRatio->cd(1); h1_Hit_UsedRatio_Total->Draw("hist");
    c1_Hit_UsedRatio->cd(2); h1_Hit_UsedRatio_1Pad->Draw("hist"); h1_Hit_UsedRatio_1Pad->SetLineWidth(2);
    c1_Hit_UsedRatio->cd(3); 
    h1_Hit_UsedRatio_HitDensity_30Tb->Draw("hist"); h1_Hit_UsedRatio_HitDensity_30Tb->SetLineColor(2); h1_Hit_UsedRatio_HitDensity_30Tb->SetLineWidth(2);
    h1_Hit_UsedRatio_HitDensity_20Tb->Draw("samehist"); h1_Hit_UsedRatio_HitDensity_20Tb->SetLineColor(4); h1_Hit_UsedRatio_HitDensity_20Tb->SetLineWidth(2);
    h1_Hit_UsedRatio_HitDensity_10Tb->Draw("samehist"); h1_Hit_UsedRatio_HitDensity_10Tb->SetLineColor(6); h1_Hit_UsedRatio_HitDensity_10Tb->SetLineWidth(2);
    c1_Hit_UsedRatio->Update();
  }
  
  
  if(c1_MultiHit_Dis==0){ c1_MultiHit_Dis = new TCanvas("c1_MultiHit_Dis","c1_MultiHit_Dis",1); }
  else { c1_MultiHit_Dis->Clear(); }
  c1_MultiHit_Dis->Divide(1,2);
  h2_MultiHit_PadPlane->Scale(1.0/AnaEvtNum);
  h2_MultiHit_SideView->Scale(1.0/AnaEvtNum);
  c1_MultiHit_Dis->cd(1); h2_MultiHit_PadPlane->Draw("colz");
  c1_MultiHit_Dis->cd(2); h2_MultiHit_SideView->Draw("colz");
  
  if(c1_TrackResErr==0) { c1_TrackResErr = new TCanvas("c1_TrackResErr","c1_TrackResErr",1); }
  else { c1_TrackResErr->Clear(); }
//  c1_TrackResErr->Divide(1,2);
//  c1_TrackResErr->cd(1);
  h1_TrackResErr->Draw("hist"); h1_TrackResErr->SetLineWidth(2);
//  c1_TrackResErr->cd(2);
//  h2_TrackResErr_HitDensity->Draw("colz");

  if(c1_Saturated_FollowHit==0) { c1_Saturated_FollowHit = new TCanvas("c1_Saturated_FollowHit","c1_Saturated_FollowHit",1); }
  else { c1_Saturated_FollowHit->Clear(); }
  c1_Saturated_FollowHit->cd();
  h2_Saturated_FollowHit->Draw();
  cout<<"SaturatedPadNum: "<<SaturatedPadNum<<endl;
}
