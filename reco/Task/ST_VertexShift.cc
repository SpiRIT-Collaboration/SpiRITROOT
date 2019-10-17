#include "ST_VertexShift.hh"
ClassImp(ST_VertexShift);

ST_VertexShift::ST_VertexShift()
{
  PhiNum = 0;
  for(int i=0;i<MaxPhiNum_ST_VertexShift;i++)
  {
    h2_Theta_TargetPosX[i] = 0;
    h2_Theta_TargetPosY[i] = 0;
  
    Profile_Theta_TargetPosX[i] = 0;
    Profile_Theta_TargetPosY[i] = 0;
  
    h1_Theta_TargetPosX_Smooth[i] = 0;
    h1_Theta_TargetPosY_Smooth[i] = 0;
  }
  Threshold_Remove_BG = 1;
  Opt_Remove_BG = 0;
  ThetaCountClear_Fraction = 0.5; //only the first half bin will be applied count clearance.
}

ST_VertexShift::~ST_VertexShift()
{;}

double ST_VertexShift::Get_Corrected_DistToVertex_OnTarget(double Theta,double Phi, TVector3 PosOnTarget, TVector3 PosVertexOnTarget)
{
  int PhiIndex = Phi/10; //here, the Phi belong to (0,360).
  int ThetaIndex = Theta/1;
  double ShiftX = Profile_Theta_TargetPosX[PhiIndex]->GetBinContent(ThetaIndex);
  double ShiftY = Profile_Theta_TargetPosY[PhiIndex]->GetBinContent(ThetaIndex);
  double DistX = (PosOnTarget-PosVertexOnTarget).X()-ShiftX;
  double DistY = (PosOnTarget-PosVertexOnTarget).Y()-ShiftY;

return Sqrt(DistX*DistX + DistY*DistY);
}

void ST_VertexShift::GetShiftValue(double Theta,double Phi,double* ShiftX,double* ShiftY)
{
  int PhiIndex = Phi/10; //here, the Phi belong to (0,360).
  int ThetaIndex = Theta/1;
  *ShiftX = Profile_Theta_TargetPosX[PhiIndex]->GetBinContent(ThetaIndex);
  *ShiftY = Profile_Theta_TargetPosY[PhiIndex]->GetBinContent(ThetaIndex);
}

void ST_VertexShift::GetShiftValue_Smooth(double Theta,double Phi,double* ShiftX,double* ShiftY)
{
  int PhiIndex = Phi/10; //here, the Phi belong to (0,360).
  int ThetaIndex = Theta/1;
  *ShiftX = h1_Theta_TargetPosX_Smooth[PhiIndex]->GetBinContent(ThetaIndex);
  *ShiftY = h1_Theta_TargetPosY_Smooth[PhiIndex]->GetBinContent(ThetaIndex);
}


void ST_VertexShift::Load_BDC_Correction(string FileName)
{ //this function will load the profile and histogram directly.
  TFile* f1_CorrectionData = new TFile(FileName.c_str());
  PhiNum = 36; // here I will always suppose it is 36 now.
  int PhiRange[MaxPhiNum_ST_VertexShift][2];
  int ThetaRange[MaxPhiNum_ST_VertexShift][2]; //[0]: X, [1]: Y.
  for(int i=0;i<PhiNum;i++)
  {
    PhiRange[i][0] = i*10;
    PhiRange[i][1] = i*10+10;
  }
  
  char NameTem[200];
  for(int i=0;i<PhiNum;i++)
  {
    // the below is for loading the PosX
    sprintf(NameTem,"h2_Theta_TargetPosX_Phi_%d_%d",PhiRange[i][0],PhiRange[i][1]);
    h2_Theta_TargetPosX[i] = (TH2D*) f1_CorrectionData->Get(NameTem);
    
    sprintf(NameTem,"h2_Theta_TargetPosX_Phi_%d_%d_pfx",PhiRange[i][0],PhiRange[i][1]);
    Profile_Theta_TargetPosX[i] = (TProfile*) f1_CorrectionData->Get(NameTem);
    
    sprintf(NameTem,"h2_Theta_TargetPosX_Phi_%d_%d_pfx_Smooth",PhiRange[i][0],PhiRange[i][1]);
    h1_Theta_TargetPosX_Smooth[i] = (TH1D*) f1_CorrectionData->Get(NameTem);
    if(h1_Theta_TargetPosX_Smooth[i]==0) { cout<<NameTem<<" Load failed! "<<endl; }
    
    // the below is for loading the PosY
    sprintf(NameTem,"h2_Theta_TargetPosY_Phi_%d_%d",PhiRange[i][0],PhiRange[i][1]);
    h2_Theta_TargetPosY[i] = (TH2D*) f1_CorrectionData->Get(NameTem);
    
    sprintf(NameTem,"h2_Theta_TargetPosY_Phi_%d_%d_pfx",PhiRange[i][0],PhiRange[i][1]);
    Profile_Theta_TargetPosY[i] = (TProfile*) f1_CorrectionData->Get(NameTem);
    
    sprintf(NameTem,"h2_Theta_TargetPosY_Phi_%d_%d_pfx_Smooth",PhiRange[i][0],PhiRange[i][1]);
    h1_Theta_TargetPosY_Smooth[i] = (TH1D*) f1_CorrectionData->Get(NameTem);
    if(h1_Theta_TargetPosY_Smooth[i]==0) { cout<<NameTem<<" Load failed! "<<endl; }
  }
}

//this function can calculate the profile and histogram based on the 2D histogram( theta, distx/disty ).
void ST_VertexShift::Cal_BDC_Correction(string FileName)
{
  TFile* f1_CorrectionData = new TFile(FileName.c_str(),"update");
  PhiNum = 36; // here I will always suppose it is 36 now.
  int PhiRange[MaxPhiNum_ST_VertexShift][2];
  int ThetaRange[MaxPhiNum_ST_VertexShift][2]; //[0]: X, [1]: Y.
  for(int i=0;i<PhiNum;i++)
  {
    PhiRange[i][0] = i*10;
    PhiRange[i][1] = i*10+10;
  }
  
  char NameTem[200];
  
  for(int i=0;i<PhiNum;i++)
  {
    // the below is for the PosX
    sprintf(NameTem,"h2_Theta_TargetPosX_Phi_%d_%d",PhiRange[i][0],PhiRange[i][1]);
    h2_Theta_TargetPosX[i] = (TH2D*) f1_CorrectionData->Get(NameTem);
    if(h2_Theta_TargetPosX[i]==0) { cout<<"Error when read TH2D: "<<NameTem<<endl; continue;}
    
    sprintf(NameTem,"h2_Theta_TargetPosX_Phi_%d_%d_pfx",PhiRange[i][0],PhiRange[i][1]);
    Clean_Count(h2_Theta_TargetPosX[i]);
    Profile_Theta_TargetPosX[i] = (TProfile*) h2_Theta_TargetPosX[i]->ProfileX(NameTem);
    
    sprintf(NameTem,"h2_Theta_TargetPosX_Phi_%d_%d_pfx_Smooth",PhiRange[i][0],PhiRange[i][1]);
    int XBinNum = Profile_Theta_TargetPosX[i]->GetNbinsX();
    h1_Theta_TargetPosX_Smooth[i] = new TH1D(NameTem,NameTem,XBinNum,0,90);
    for(int j=1;j<=XBinNum;j++)
    {
      h1_Theta_TargetPosX_Smooth[i]->SetBinContent(j,Profile_Theta_TargetPosX[i]->GetBinContent(j));
    }
    h1_Theta_TargetPosX_Smooth[i]->Smooth(1);
    Profile_Theta_TargetPosX[i]->Write("",TObject::kOverwrite);
    h1_Theta_TargetPosX_Smooth[i]->Write("",TObject::kOverwrite);
    
    // the below is for the PosY
    sprintf(NameTem,"h2_Theta_TargetPosY_Phi_%d_%d",PhiRange[i][0],PhiRange[i][1]);
    h2_Theta_TargetPosY[i] = (TH2D*) f1_CorrectionData->Get(NameTem);
    if(h2_Theta_TargetPosY[i]==0) { cout<<"Error when read TH2D: "<<NameTem<<endl; continue;}
    
    sprintf(NameTem,"h2_Theta_TargetPosY_Phi_%d_%d_pfx",PhiRange[i][0],PhiRange[i][1]);
    Clean_Count(h2_Theta_TargetPosY[i]);
    Profile_Theta_TargetPosY[i] = (TProfile*) h2_Theta_TargetPosY[i]->ProfileX(NameTem);
    
    sprintf(NameTem,"h2_Theta_TargetPosY_Phi_%d_%d_pfx_Smooth",PhiRange[i][0],PhiRange[i][1]);
    XBinNum = Profile_Theta_TargetPosY[i]->GetNbinsX();
    h1_Theta_TargetPosY_Smooth[i] = new TH1D(NameTem,NameTem,XBinNum,0,90);
    for(int j=1;j<=XBinNum;j++)
    {
      h1_Theta_TargetPosY_Smooth[i]->SetBinContent(j,Profile_Theta_TargetPosY[i]->GetBinContent(j));
    }
    h1_Theta_TargetPosY_Smooth[i]->Smooth(1);
    Profile_Theta_TargetPosY[i]->Write("",TObject::kOverwrite);
    h1_Theta_TargetPosY_Smooth[i]->Write("",TObject::kOverwrite);
  }
}


//remove the small count to get a more accurate center value, but do not change the bin content in the root file.
void ST_VertexShift::Clean_Count(TH2D* h2_tem)
{
  int XbinNum = h2_tem->GetNbinsX();
  int YbinNum = h2_tem->GetNbinsY();
  
  int XbinNum_forClean = XbinNum;
  
  if(Opt_Remove_BG==0) { XbinNum_forClean = XbinNum; } //clean all the count that is less than the threshold(Threshold_Remove_BG).
  else if(Opt_Remove_BG==1) { XbinNum_forClean = ThetaCountClear_Fraction*XbinNum; }
  
  for(int iX=1;iX<=XbinNum_forClean;iX++)
  {
    for(int iY=1;iY<=YbinNum;iY++)
    {
      double Content = h2_tem->GetBinContent(iX,iY);
      if(Content<=Threshold_Remove_BG) { h2_tem->SetBinContent(iX,iY,0); }
    }
  }
}

void ST_VertexShift::Set_ThetaCountClear_Fraction(double tem)
{ 
  if( tem>1 || tem<=0 ) { cout<<"The count clear fraction should be belong to (0,1)! The value that you set failed!"<<endl; getchar(); }
  ThetaCountClear_Fraction = tem;
}
