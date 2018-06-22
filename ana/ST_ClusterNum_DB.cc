#include "ST_ClusterNum_DB.hh"
ClassImp(ST_ClusterNum_DB);

ST_ClusterNum_DB::ST_ClusterNum_DB()
{
  IsDebug = 0;
  
  MomentumNum_Plus = 0;
  MomentumNum_Minus = 0;
  ThetaBin_Unit = 90.0/THETANUM;
  PhiBin_Unit = 360.0/PHINUM;
  
  // the below is the default value of momentum range.
  Momentum_Range_Plus[0] = 50;
  Momentum_Range_Plus[1] = 3000;
  Momentum_Range_Minus[0] = 50;
  Momentum_Range_Minus[1] = 1000;
  
  for(int iMomentum=0;iMomentum<MaxMomentumNum_Plus;iMomentum++)
  { MomentumValue_Plus[iMomentum] = -1; }
  
  for(int iMomentum=0;iMomentum<MaxMomentumNum_Minus;iMomentum++)
  { MomentumValue_Minus[iMomentum] = -1; }
}

ST_ClusterNum_DB::~ST_ClusterNum_DB()
{;}

void ST_ClusterNum_DB::Initial_Config(string FileName_Tem)
{
  cout<<"--> Reading File: "<<FileName_Tem<<" ..."<<endl;
  ifstream infile(FileName_Tem.c_str());
  MomentumNum_Plus = 0;
  MomentumNum_Minus = 0;
  
  double Momentum_Tem = 0; double Charge_Tem = 0;
  char buf[1000];
  int buflen = 1000;
  
  while(infile)
  {
    infile.clear(ios::goodbit);
    infile.getline(buf,buflen);
//    cout<<buf<<endl;
    if(buf[0]=='!' || buf[0]=='#' || buf[0]==' ' || strlen(buf)<=1) { continue; }
    istringstream buf_stream(&(buf[0]));
    buf_stream>>Momentum_Tem>>Charge_Tem;
//    cout<<Momentum_Tem<<"  "<<Charge_Tem<<endl;
    if(Charge_Tem==1)
    {
      MomentumValue_Plus[MomentumNum_Plus] = Momentum_Tem;
      MomentumNum_Plus++;
    }
    else if(Charge_Tem==-1)
    {
      MomentumValue_Minus[MomentumNum_Minus] = Momentum_Tem;
      MomentumNum_Minus++;
    }
    else
    {
      cout<<"Except 1 and -1, the other charge is not accepted!"<<endl;
    }
  }
  cout<<"Plus Charge MomentumNum: "<<MomentumNum_Plus<<"    Minus Charge MomentumNum: "<<MomentumNum_Minus<<endl;
}

void ST_ClusterNum_DB::ReadDB(string FileName_Tem)
{
  f1_DB_ClusterNum = new TFile(FileName_Tem.c_str(),"Update");
  char NameTem[200];
  cout<<"--> Reading File: "<<FileName_Tem<<" ..."<<endl;
  cout<<"--->Download DB for positive charge!"<<endl;
  for(int iMomentum=0;iMomentum<MomentumNum_Plus;iMomentum++)
  {
    int Charge_DB_Tem = -1;
    double Momentum_DB_Tem = 0;
    double ThetaPhi_DB_Tem[THETANUM][PHINUM] = {{0}};
    
    sprintf(NameTem,"t1_DB_ClusterNum_Charge%d_Momentum%.2f",1,MomentumValue_Plus[iMomentum]);
    if(IsDebug==1) { cout<<"Read TTree: "<<NameTem<<endl; }
    TTree* t1_DB_Tem = (TTree*) f1_DB_ClusterNum->Get(NameTem);
    
    t1_DB_Tem->SetBranchAddress("Charge",&Charge_DB_Tem);
    t1_DB_Tem->SetBranchAddress("Momentum",&Momentum_DB_Tem);
    t1_DB_Tem->SetBranchAddress("ThetaPhi_DB",ThetaPhi_DB_Tem);
    
    int DBTree_EntryNum = t1_DB_Tem->GetEntries();
    
    if(DBTree_EntryNum!=1) { cout<<"For momentum "<<MomentumValue_Plus[iMomentum]<<". The DB tree has "<<DBTree_EntryNum<<" entries! Check it!"<<endl; getchar(); continue;}
    t1_DB_Tem->GetEntry(0);
    if(Charge_DB_Tem!=1) { cout<<"For Momentum "<<MomentumValue_Plus[iMomentum]<<". The charge is not +1, Check it"<<endl; getchar(); continue; }
    if(Momentum_DB_Tem!=MomentumValue_Plus[iMomentum]) { cout<<"For Momentum "<<MomentumValue_Plus[iMomentum]<<". The momentum is not same, Check it"<<endl; getchar(); continue; }
    
    for(int iTheta=0; iTheta<THETANUM;iTheta++)
    {
      for(int iPhi=0;iPhi<PHINUM;iPhi++)
      {
        ClusterNum_Plus[iTheta][iPhi][iMomentum] = ThetaPhi_DB_Tem[iTheta][iPhi];
      }
    }
    t1_DB_Tem->Delete(); t1_DB_Tem = 0;
  }
  
  
  cout<<"--->Download DB for negative charge!"<<endl;
  for(int iMomentum=0;iMomentum<MomentumNum_Minus;iMomentum++)
  {
    int Charge_DB_Tem = -1;
    double Momentum_DB_Tem = 0;
    double ThetaPhi_DB_Tem[THETANUM][PHINUM] = {{0}};
    
    sprintf(NameTem,"t1_DB_ClusterNum_Charge%d_Momentum%.2f",-1,MomentumValue_Minus[iMomentum]);
    if(IsDebug==1) { cout<<"Read TTree: "<<NameTem<<endl; }
    TTree* t1_DB_Tem = (TTree*) f1_DB_ClusterNum->Get(NameTem);
    
    t1_DB_Tem->SetBranchAddress("Charge",&Charge_DB_Tem);
    t1_DB_Tem->SetBranchAddress("Momentum",&Momentum_DB_Tem);
    t1_DB_Tem->SetBranchAddress("ThetaPhi_DB",ThetaPhi_DB_Tem);
    
    int DBTree_EntryNum = t1_DB_Tem->GetEntries();
    
    if(DBTree_EntryNum!=1) { cout<<"For momentum "<<MomentumValue_Minus[iMomentum]<<". The DB tree has "<<DBTree_EntryNum<<" entries! Check it!"<<endl; getchar(); continue;}
    t1_DB_Tem->GetEntry(0);
    if(Charge_DB_Tem!=-1) { cout<<"For Momentum "<<MomentumValue_Minus[iMomentum]<<". The charge is not -1, Check it"<<endl; getchar(); continue; }
    if(Momentum_DB_Tem!=MomentumValue_Minus[iMomentum]) { cout<<"For Momentum "<<MomentumValue_Minus[iMomentum]<<". The momentum is not same, Check it"<<endl; getchar(); continue; }
    
    for(int iTheta=0; iTheta<THETANUM;iTheta++)
    {
      for(int iPhi=0;iPhi<PHINUM;iPhi++)
      {
        ClusterNum_Minus[iTheta][iPhi][iMomentum] = ThetaPhi_DB_Tem[iTheta][iPhi];
      }
    }
    t1_DB_Tem->Delete(); t1_DB_Tem = 0;
  }
  
}

double ST_ClusterNum_DB::GetClusterNum(int Charge_Tem, double Theta_Tem, double Phi_Tem, double Momentum_Tem)
{
  if(Charge_Tem>0 && (Momentum_Tem/Charge_Tem>Momentum_Range_Plus[1] || Momentum_Tem/Charge_Tem<Momentum_Range_Plus[0])) 
  {
    if(IsDebug==1) { cout<<"Current momentum: "<<Momentum_Tem<<" Charge: "<<Charge_Tem<<", the range of positive particel :["<<Momentum_Range_Plus[0]<<" , "<<Momentum_Range_Plus[1]<<")."<<endl; }
    return 0; 
  }
  
  if(Charge_Tem<0 && (Momentum_Tem/fabs(Charge_Tem)>Momentum_Range_Minus[1] || Momentum_Tem/fabs(Charge_Tem)<Momentum_Range_Minus[0])) 
  { 
    if(IsDebug==1) { cout<<"Current momentum: "<<Momentum_Tem<<" Charge: "<<Charge_Tem<<", the range of negative particel :["<<Momentum_Range_Minus[0]<<" , "<<Momentum_Range_Minus[1]<<")."<<endl; }
    return 0; 
  }
  
  if(Theta_Tem>90 || Theta_Tem<0) 
  { 
    if(IsDebug==1) { cout<<"Theta = "<<Theta_Tem<<", which is already out of the range in the DB! => (0,90) is the range in the DB."<<endl; }
    return 0;
  }
  
  if(Phi_Tem>180 || Phi_Tem<-180) 
  { 
    if(IsDebug==1) { cout<<"Phi = "<<Phi_Tem<< ", which is already out of the range in the DB! => (-180,180) is the range in the DB."<<endl; }
    return 0;
  }
  
  Momentum_Tem = fabs(Momentum_Tem/Charge_Tem);
  
  int ThetaIndex = (int) (Theta_Tem/ThetaBin_Unit);
  int PhiIndex = (int) ((Phi_Tem+180)/PhiBin_Unit);
//  cout<<"ThetaIndex: "<<ThetaIndex<<"  PhiIndex:  "<<PhiIndex<<endl;
  
  if(Charge_Tem>0)
  {
    for(int i=0;i<MomentumNum_Plus;i++)
    {
      if(Momentum_Tem==MomentumValue_Plus[i]) { return ClusterNum_Plus[ThetaIndex][PhiIndex][i]; }
      else if(Momentum_Tem>MomentumValue_Plus[i] && i+1<MomentumNum_Plus && Momentum_Tem<MomentumValue_Plus[i+1]) 
      {// the linear interpolation
        double k = (Momentum_Tem-MomentumValue_Plus[i])/(MomentumValue_Plus[i+1]-Momentum_Tem);
        double n1 = ClusterNum_Plus[ThetaIndex][PhiIndex][i];
        double n2 = ClusterNum_Plus[ThetaIndex][PhiIndex][i+1];
        return ((k*n2+n1)/(1+k));
      }
    }
    cout<<Momentum_Tem<<" MeV/c is not in the range of DB"<<endl; 
    return 0;
  }
  else if(Charge_Tem<0)
  {
    for(int i=0;i<MomentumNum_Minus;i++)
    {
      if(Momentum_Tem==MomentumValue_Minus[i]) { return ClusterNum_Minus[ThetaIndex][PhiIndex][i]; }
      else if(Momentum_Tem>MomentumValue_Minus[i] && i+1<MomentumNum_Minus && Momentum_Tem<MomentumValue_Minus[i+1]) 
      {// the linear interpolation
        double k = (Momentum_Tem-MomentumValue_Minus[i])/(MomentumValue_Minus[i+1]-Momentum_Tem);
        double n1 = ClusterNum_Minus[ThetaIndex][PhiIndex][i];
        double n2 = ClusterNum_Minus[ThetaIndex][PhiIndex][i+1];
        return ((k*n2+n1)/(1+k));
      }
    }
    cout<<Momentum_Tem<<" MeV/c is not in the range of DB"<<endl; 
    return 0;
  }
  else
  {
     if(IsDebug==1) { cout<<"Charge "<<Charge_Tem<<" Not Existed in the DB!"<<endl; return 0; }
  }
}
// most of time we want to input the momentum vector
double ST_ClusterNum_DB::GetClusterNum(int Charge_Tem, TVector3 Momentum_Tem)
{
  double Momentum_Mag = Momentum_Tem.Mag();
  double Theta_Tem = Momentum_Tem.Theta()*180.0/Pi();
  if(Theta_Tem>90)
  {
    if(IsDebug==1)
    {
      cout<<"Theta > 90"<<endl;
      cout<<Momentum_Tem.X()<<"  "<<Momentum_Tem.Y()<<"  "<<Momentum_Tem.Z()<<endl;
    }
    return 0; 
  }
  double Phi_Tem = Momentum_Tem.Phi()*180.0/Pi(); //here the Phi_tem belong to (-180,180)
//  if(IsDebug==1)  { cout<<Momentum_Mag<<"  "<<Theta_Tem<<"  "<<Phi_Tem<<endl; }
  return GetClusterNum(Charge_Tem, Theta_Tem, Phi_Tem, Momentum_Mag);
}



//this function can be used to show the clusterNum on the (Theta, Phi)
void ST_ClusterNum_DB::Draw_Theta_Phi_Dis(int Charge_Tem, double Momentum_Tem, int Opt_interpolation)
{
  if(Charge_Tem>0 && (Momentum_Tem>Momentum_Range_Plus[1] || Momentum_Tem<Momentum_Range_Plus[0])) 
  { cout<<"The range of positive particel :["<<Momentum_Range_Plus[0]<<" , "<<Momentum_Range_Plus[1]<<")."<<endl; return; }
  if(Charge_Tem<0 && (Momentum_Tem>Momentum_Range_Minus[1] || Momentum_Tem<Momentum_Range_Minus[0])) 
  { cout<<"The range of negative particel :["<<Momentum_Range_Minus[0]<<" , "<<Momentum_Range_Minus[1]<<")."<<endl; return; }
  
  //Find the relative charge and momentum
  int MomentumIndex = -1;
  
  if(Charge_Tem==1)
  {
    for(int i=0;i<MomentumNum_Plus;i++)
    { if(Momentum_Tem==MomentumValue_Plus[i]){ MomentumIndex = i; } }
  }
  else if(Charge_Tem==-1)
  {
    for(int i=0;i<MomentumNum_Minus;i++)
    { if(Momentum_Tem==MomentumValue_Minus[i]){ MomentumIndex = i; } }
  }
  else
  { cout<<"Charge "<<Charge_Tem<<" Not Existed!"<<endl; return; }
  if(MomentumIndex==-1 && Opt_interpolation == 0) { cout<<"Charge = "<<Charge_Tem<<" Momentum = "<<Momentum_Tem<<" not Existed in the DB!"<<endl; return; }
  else if(MomentumIndex==-1 && Opt_interpolation ==1) { cout<<"Charge = "<<Charge_Tem<<" Momentum = "<<Momentum_Tem<<" not Existed in the DB! Interpolation is used! "<<endl; }
  
//  cout<<"MomentumIndex: "<<MomentumIndex<<endl;
  
  //for drawing
  char NameTem[200];
  sprintf(NameTem,"c1_ThetaPhi_ClusterNum_Dis_%.2fMeV",Momentum_Tem);
  TCanvas* c1_ThetaPhi_ClusterNum_Dis = new TCanvas(NameTem,NameTem,600,600);
    
  sprintf(NameTem,"h2_ThetaPhi_ClusterNum_Dis_Charge%d_%.2fMeV",Charge_Tem,Momentum_Tem);
  TH2D* h2_ThetaPhi_ClusterNum_Dis = new TH2D(NameTem,NameTem,THETANUM,0,90,PHINUM,-180,180);
  
  for(int iTheta=0; iTheta<THETANUM;iTheta++)
  {
    for(int iPhi=0;iPhi<PHINUM;iPhi++)
    {
      if(Charge_Tem==-1 && MomentumIndex!=-1) { h2_ThetaPhi_ClusterNum_Dis->SetBinContent(iTheta+1,iPhi+1,ClusterNum_Minus[iTheta][iPhi][MomentumIndex]); }
      else if(Charge_Tem==1 && MomentumIndex!=-1) { h2_ThetaPhi_ClusterNum_Dis->SetBinContent(iTheta+1,iPhi+1,ClusterNum_Plus[iTheta][iPhi][MomentumIndex]); }
      else if(MomentumIndex==-1 && Opt_interpolation==1)
      {
        h2_ThetaPhi_ClusterNum_Dis->SetBinContent(iTheta+1,iPhi+1,GetClusterNum(Charge_Tem,iTheta*ThetaBin_Unit,iPhi*PhiBin_Unit-180,Momentum_Tem));
      }
    }
  }
  
  c1_ThetaPhi_ClusterNum_Dis->cd();
  h2_ThetaPhi_ClusterNum_Dis->Draw("colz");
}


//this function allows us to draw the ClusterNum for different momentum, but same direction.
void ST_ClusterNum_DB::Draw_withMomentum_Dis(double Theta_Tem,double Phi_Tem,int Charge_Tem, int MomentumNum_Tem,double* MomentumValue_Tem)
{
  char NameTem[200];
  sprintf(NameTem,"c1_ClusterNum_Dis_alongMomentum_Theta%.2f_Phi%.2f",Theta_Tem,Phi_Tem);
  TCanvas* c1_ClusterNum_Dis_alongMomentum = new TCanvas(NameTem,NameTem,1);
  double ClusterNum_Tem[1000] = {0}; // here I suppose the num of momentum cannot be larger than 1000;
  for(int iMomentum=0;iMomentum<MomentumNum_Tem;iMomentum++) 
  {
    ClusterNum_Tem[iMomentum] = GetClusterNum(Charge_Tem,Theta_Tem,Phi_Tem,MomentumValue_Tem[iMomentum]);
  }
  TGraph* gr1_tem = new TGraph(MomentumNum_Tem,MomentumValue_Tem,ClusterNum_Tem);
  gr1_tem->Draw("AL*");
}

TGraph* ST_ClusterNum_DB::Draw_withMomentum_Dis(double Theta_Tem,double Phi_Tem,int Charge_Tem,int DrawOpt)
{
  char NameTem[200];
  
  int MomentumNum_Tem = 0;
  if(Charge_Tem==1) { MomentumNum_Tem = MomentumNum_Plus; }
  else if(Charge_Tem==-1) { MomentumNum_Tem = MomentumNum_Minus; }
  else { cout<<"Charge: "<<Charge_Tem<<" .No Way! "<<endl; return 0;}
  
  double ClusterNum_Tem[1000] = {0}; // here I suppose the num of momentum cannot be larger than 1000;
  double MomentumValue_Tem[1000] = {0};
  
  for(int iMomentum=0;iMomentum<MomentumNum_Tem;iMomentum++) 
  {
    ClusterNum_Tem[iMomentum] = GetClusterNum(Charge_Tem,Theta_Tem,Phi_Tem,MomentumValue_Plus[iMomentum]);
    MomentumValue_Tem[iMomentum] = MomentumValue_Plus[iMomentum];
  }
  
  TGraph* gr1_tem = new TGraph(MomentumNum_Tem,MomentumValue_Tem,ClusterNum_Tem); 
  if(DrawOpt!=0)
  {
    sprintf(NameTem,"c1_ClusterNum_Dis_alongMomentum_Theta%.2f_Phi%.2f",Theta_Tem,Phi_Tem);
    TCanvas* c1_ClusterNum_Dis_alongMomentum = new TCanvas(NameTem,NameTem,1);
    gr1_tem->Draw("AL*");
  }
  
return gr1_tem;
}
