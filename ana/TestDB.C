void TestDB()
{
  ST_ClusterNum_DB* db = new ST_ClusterNum_DB();

  db->Initial_Config("Momentum.config");
  db->ReadDB("f1_DB_ClusterNum.root");
  double Momentum_Range_Plus[2] = {50,3000};
  double Momentum_Range_Minus[2] = {50,1000};
  db->Set_MomentumRange_Plus(Momentum_Range_Plus);
  db->Set_MomentumRange_Minus(Momentum_Range_Minus);
  
  db->Draw_Theta_Phi_Dis(1, 100,0);
  db->Draw_Theta_Phi_Dis(1, 500,0);
  db->Draw_Theta_Phi_Dis(1, 1000,0);
  db->Draw_Theta_Phi_Dis(1, 2000,0);
  db->Draw_Theta_Phi_Dis(1, 2500,0);
/*
  db->Draw_Theta_Phi_Dis(1, 100,0);
  db->Draw_Theta_Phi_Dis(1, 110,0);
  db->Draw_Theta_Phi_Dis(1, 120,0);
  db->Draw_Theta_Phi_Dis(1, 130,0);
  db->Draw_Theta_Phi_Dis(1, 140,0);
  db->Draw_Theta_Phi_Dis(1, 150,0);
*/
/*
  db->Draw_Theta_Phi_Dis(1, 60,0);
  db->Draw_Theta_Phi_Dis(1, 57.5,0);
  db->Draw_Theta_Phi_Dis(1, 55,0);
  db->Draw_Theta_Phi_Dis(1, 50,0);
  db->Draw_Theta_Phi_Dis(1, 52.5,0);
*/
  int Charge = 1;
  double Theta = 75;
  double Phi = 10;
  double Momentum_1 = 500;
  int ClusterNum = db->GetClusterNum(Charge, Theta, Phi, Momentum_1);
  cout<<ClusterNum<<endl;
  TVector3 P_Tem(0,0,1);
  P_Tem.SetMag(Momentum_1); P_Tem.SetTheta(Theta*Pi()/180); P_Tem.SetPhi(Phi*Pi()/180);
//  cout<<P_Tem.Mag()<<"  "<<P_Tem.Theta()*180/Pi()<<"  "<<P_Tem.Phi()*180/Pi()<<endl;
  ClusterNum = db->GetClusterNum(Charge,P_Tem);
  cout<<ClusterNum<<endl;
  
  const int MomentumNum_Tem = 24;
  double MomentumValue[]={51,53.5,54,59,61,62.5,65.5,67,71,74,75,81,89,92,96,113,158,212,247,510,1070,1370,2009,2450};
  
  db->Draw_withMomentum_Dis(Theta,Phi,Charge,MomentumNum_Tem,MomentumValue);
/*  
  Theta = 45; Phi = 0;
  db->Draw_withMomentum_Dis(Theta,Phi,Charge,1);
*/
  Theta = 75; Phi = 10;
  TGraph* gr1_tem = db->Draw_withMomentum_Dis(Theta,Phi,Charge,0);
  TCanvas* c1_tem = new TCanvas("c1_tem","c1_tem",1);
  c1_tem->cd();
  gr1_tem->Draw("AL*");
  
  TGraph* gr1_Array[3] = {0};
  double Theta_Array[3] = {20,22,24};
  double Phi_Array[3] = {175,175,175};
  TCanvas* c1_Diff_Theta = new TCanvas("c1_Diff_Theta","c1_Diff_Theta",1);
  TLegend* legend = new TLegend(0.6,0.6,0.8,0.8);
  for(int i=0;i<3;i++)
  {
    c1_Diff_Theta->cd(i+1);
    gr1_Array[i] = db->Draw_withMomentum_Dis(Theta_Array[i],Phi_Array[i],1,0);
    if( i==0 ) { gr1_Array[i]->Draw("AL*"); gr1_Array[i]->GetYaxis()->SetRangeUser(1,100); }
    else { gr1_Array[i]->Draw("sameL*"); }
    char NameTem[200];
    sprintf(NameTem,"Theta%.1f",Theta_Array[i]);
    legend->AddEntry(gr1_Array[i],NameTem,"lp");
  }
  legend->Draw();
  
  Theta = 20; Phi = -100;
  db->Draw_withMomentum_Dis(Theta,Phi,Charge,1);
}
