void Test_ST_VertexShift()
{
  ST_VertexShift* VertexShifter = new ST_VertexShift();
//  VertexShifter->Cal_BDC_Correction("f1_BDCCorrection_Theta_TargetPos.root");
  VertexShifter->Load_BDC_Correction("f1_BDCCorrection_Theta_TargetPos.root");
  
  int ClusterNum = 36;
  //the below is for drawing.
  TCanvas* c1_Theta_TargetPosX = new TCanvas("c1_Theta_TargetPosX","c1_Theta_TargetPosX",1);
  c1_Theta_TargetPosX->Divide(6,6);
  
  TCanvas* c1_Theta_TargetPosY = new TCanvas("c1_Theta_TargetPosY","c1_Theta_TargetPosY",1);
  c1_Theta_TargetPosY->Divide(6,6);
  
  for(int i=0;i<ClusterNum;i++)
  {
    c1_Theta_TargetPosX->cd(i+1);
    TH2D* h2_Theta_TargetPosX = VertexShifter->Get_Theta_TargetPosX(i);
    h2_Theta_TargetPosX->Draw("colz");
    TProfile* Profile_PosX = VertexShifter->GetProfileX(i);
    Profile_PosX->Draw("same");
    TH1D* h1_PosX = VertexShifter->GetProfileX_Smooth(i);
    h1_PosX->Draw("same");
    
    c1_Theta_TargetPosY->cd(i+1);
    TH2D* h2_Theta_TargetPosY = VertexShifter->Get_Theta_TargetPosY(i);
    h2_Theta_TargetPosY->Draw("colz");
    TProfile* Profile_PosY = VertexShifter->GetProfileY(i);
    Profile_PosY->Draw("same");
    TH1D* h1_PosY = VertexShifter->GetProfileY_Smooth(i);
    h1_PosY->Draw("same");
  }  
}
