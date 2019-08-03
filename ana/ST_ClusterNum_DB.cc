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
  Momentum_Range_Plus[1] = 4000;
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
  cout<<"Plus Charge MomentumNum: "<<MomentumNum_Plus<<" Range: ("<<Momentum_Range_Plus[0]<<","<<Momentum_Range_Plus[1]<<")"<<endl
      <<"Minus Charge MomentumNum: "<<MomentumNum_Minus<<" Range: ("<<Momentum_Range_Minus[0]<<","<<Momentum_Range_Minus[1]<<")"<<endl;
}

void ST_ClusterNum_DB::ReadDB(string FileName_Tem)
{
  f1_DB_ClusterNum = new TFile(FileName_Tem.c_str(),"read");
  char NameTem[200];
  cout<<"--> Reading File: "<<FileName_Tem<<" ..."<<endl;
  cout<<"--->Download DB for positive charge!"<<endl;
  for(int iMomentum=0;iMomentum<MomentumNum_Plus;iMomentum++)
  {
    int Charge_DB_Tem = -1;
    double Momentum_DB_Tem = 0;
    double ClusterNum_AlongX_Tem[THETANUM][PHINUM] = {{0}};
    double ClusterNum_AlongZ_Tem[THETANUM][PHINUM] = {{0}};
    double TrackLength_XZ_Tem[THETANUM][PHINUM] = {{0}};
    double TrackLength_3D_Tem[THETANUM][PHINUM] = {{0}};
    double dEdX_Avg_Tem[THETANUM][PHINUM] = {{0}};
    double dEdX_Truncated_Tem[THETANUM][PHINUM] = {{0}};
    
    sprintf(NameTem,"t1_DB_ClusterNum_Charge%d_Momentum%.2f",1,MomentumValue_Plus[iMomentum]);
    if(IsDebug==1) { cout<<"Read TTree: "<<NameTem<<endl; }
    TTree* t1_DB_Tem = (TTree*) f1_DB_ClusterNum->Get(NameTem);
    
    t1_DB_Tem->SetBranchAddress("Charge",&Charge_DB_Tem);
    t1_DB_Tem->SetBranchAddress("Momentum",&Momentum_DB_Tem);
    t1_DB_Tem->SetBranchAddress("ClusterNum_AlongX",ClusterNum_AlongX_Tem);
    t1_DB_Tem->SetBranchAddress("ClusterNum_AlongZ",ClusterNum_AlongZ_Tem);
    t1_DB_Tem->SetBranchAddress("TrackLength_XZ",TrackLength_XZ_Tem);
    t1_DB_Tem->SetBranchAddress("TrackLength_3D",TrackLength_3D_Tem);
    t1_DB_Tem->SetBranchAddress("dEdX_Avg",dEdX_Avg_Tem);
    t1_DB_Tem->SetBranchAddress("dEdX_Truncated",dEdX_Truncated_Tem);
    
    int DBTree_EntryNum = t1_DB_Tem->GetEntries();
    
    if(DBTree_EntryNum!=1) { cout<<"For momentum "<<MomentumValue_Plus[iMomentum]<<". The DB tree has "<<DBTree_EntryNum<<" entries! Check it!"<<endl; getchar(); continue;}
    t1_DB_Tem->GetEntry(0);
    if(Charge_DB_Tem!=1) { cout<<"For Momentum "<<MomentumValue_Plus[iMomentum]<<". The charge is not +1, Check it"<<endl; getchar(); continue; }
    if(Momentum_DB_Tem!=MomentumValue_Plus[iMomentum]) { cout<<"For Momentum "<<MomentumValue_Plus[iMomentum]<<". The momentum is not same, Check it"<<endl; getchar(); continue; }
    
    for(int iTheta=0; iTheta<THETANUM;iTheta++)
    {
      for(int iPhi=0;iPhi<PHINUM;iPhi++)
      {
        ClusterNum_AlongX_Plus[iTheta][iPhi][iMomentum] = ClusterNum_AlongX_Tem[iTheta][iPhi];
        ClusterNum_AlongZ_Plus[iTheta][iPhi][iMomentum] = ClusterNum_AlongZ_Tem[iTheta][iPhi];
        TrackLength_XZ_Plus[iTheta][iPhi][iMomentum] = TrackLength_XZ_Tem[iTheta][iPhi];
        TrackLength_3D_Plus[iTheta][iPhi][iMomentum] = TrackLength_3D_Tem[iTheta][iPhi];
        dEdX_Avg_Plus[iTheta][iPhi][iMomentum] = dEdX_Avg_Tem[iTheta][iPhi];
        dEdX_Truncated_Plus[iTheta][iPhi][iMomentum] = dEdX_Truncated_Tem[iTheta][iPhi];
      }
    }
    t1_DB_Tem->Delete(); t1_DB_Tem = 0;
  }
  
  cout<<"--->Download DB for negative charge!"<<endl;
  for(int iMomentum=0;iMomentum<MomentumNum_Minus;iMomentum++)
  {
    int Charge_DB_Tem = -1;
    double Momentum_DB_Tem = 0;
    double ClusterNum_AlongX_Tem[THETANUM][PHINUM] = {{0}};
    double ClusterNum_AlongZ_Tem[THETANUM][PHINUM] = {{0}};
    double TrackLength_XZ_Tem[THETANUM][PHINUM] = {{0}};
    double TrackLength_3D_Tem[THETANUM][PHINUM] = {{0}};
    double dEdX_Avg_Tem[THETANUM][PHINUM] = {{0}};
    double dEdX_Truncated_Tem[THETANUM][PHINUM] = {{0}};
    
    sprintf(NameTem,"t1_DB_ClusterNum_Charge%d_Momentum%.2f",-1,MomentumValue_Minus[iMomentum]);
    if(IsDebug==1) { cout<<"Read TTree: "<<NameTem<<endl; }
    TTree* t1_DB_Tem = (TTree*) f1_DB_ClusterNum->Get(NameTem);
    
    t1_DB_Tem->SetBranchAddress("Charge",&Charge_DB_Tem);
    t1_DB_Tem->SetBranchAddress("Momentum",&Momentum_DB_Tem);
    t1_DB_Tem->SetBranchAddress("ClusterNum_AlongX",ClusterNum_AlongX_Tem);
    t1_DB_Tem->SetBranchAddress("ClusterNum_AlongZ",ClusterNum_AlongZ_Tem);
    t1_DB_Tem->SetBranchAddress("TrackLength_XZ",TrackLength_XZ_Tem);
    t1_DB_Tem->SetBranchAddress("TrackLength_3D",TrackLength_3D_Tem);
    t1_DB_Tem->SetBranchAddress("dEdX_Avg",dEdX_Avg_Tem);
    t1_DB_Tem->SetBranchAddress("dEdX_Truncated",dEdX_Truncated_Tem);
    
    int DBTree_EntryNum = t1_DB_Tem->GetEntries();
    
    if(DBTree_EntryNum!=1) { cout<<"For momentum "<<MomentumValue_Minus[iMomentum]<<". The DB tree has "<<DBTree_EntryNum<<" entries! Check it!"<<endl; getchar(); continue;}
    t1_DB_Tem->GetEntry(0);
    if(Charge_DB_Tem!=-1) { cout<<"For Momentum "<<MomentumValue_Minus[iMomentum]<<". The charge is not -1, Check it"<<endl; getchar(); continue; }
    if(Momentum_DB_Tem!=MomentumValue_Minus[iMomentum]) { cout<<"For Momentum "<<MomentumValue_Minus[iMomentum]<<". The momentum is not same, Check it"<<endl; getchar(); continue; }
    
    for(int iTheta=0; iTheta<THETANUM;iTheta++)
    {
      for(int iPhi=0;iPhi<PHINUM;iPhi++)
      {
        ClusterNum_AlongX_Minus[iTheta][iPhi][iMomentum] = ClusterNum_AlongX_Tem[iTheta][iPhi];
        ClusterNum_AlongZ_Minus[iTheta][iPhi][iMomentum] = ClusterNum_AlongZ_Tem[iTheta][iPhi];
        TrackLength_XZ_Minus[iTheta][iPhi][iMomentum] = TrackLength_XZ_Tem[iTheta][iPhi];
        TrackLength_3D_Minus[iTheta][iPhi][iMomentum] = TrackLength_3D_Tem[iTheta][iPhi];
        dEdX_Avg_Minus[iTheta][iPhi][iMomentum] = dEdX_Avg_Tem[iTheta][iPhi];
        dEdX_Truncated_Minus[iTheta][iPhi][iMomentum] = dEdX_Truncated_Tem[iTheta][iPhi];
      }
    }
    t1_DB_Tem->Delete(); t1_DB_Tem = 0;
  }
  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//the below functions are used to calculate the clusterNum, TrackLength, TrackDistance between the start and the end.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ST_ClusterNum_DB::GetClusterNum(int Charge_Tem, double Theta_Tem, double Phi_Tem, double Momentum_Tem) //the unit of angle is degree.
{
  TVector3 Momentum_Vec_Tem(0,0,Momentum_Tem);
  Momentum_Vec_Tem.SetTheta(Theta_Tem*Pi()/180.0);
  Momentum_Vec_Tem.SetPhi(Phi_Tem*Pi()/180.0);
  Momentum_Vec_Tem.SetMag(Momentum_Tem);
  
  return GetClusterNum(Charge_Tem, Momentum_Vec_Tem);
}

// most of time we want to input the momentum vector
double ST_ClusterNum_DB::GetClusterNum(int Charge_Tem, TVector3 Momentum_Tem)
{
  double Momentum_Mag = Momentum_Tem.Mag();
  double Theta_Tem = Momentum_Tem.Theta()*180.0/Pi();
  double Phi_Tem = Momentum_Tem.Phi()*180.0/Pi(); //here the Phi_tem belong to (-180,180)
  
  if(Theta_Tem>90) //90 degree
  {
    if(IsDebug==1)
    {
      cout<<"Theta > 90 degree"<<endl;
      cout<<Momentum_Tem.X()<<"  "<<Momentum_Tem.Y()<<"  "<<Momentum_Tem.Z()<<endl;
    }
    return 0;
  }
//  if(IsDebug==1)  { cout<<Momentum_Mag<<"  "<<Theta_Tem<<"  "<<Phi_Tem<<endl; }
  //the below is for checking the momentum range.
  if(Charge_Tem>0 && (Momentum_Mag/Charge_Tem>Momentum_Range_Plus[1] || Momentum_Mag/Charge_Tem<Momentum_Range_Plus[0])) 
  {
    if(IsDebug==1) { cout<<"Current momentum: "<<Momentum_Mag<<" Charge: "<<Charge_Tem<<", the range of positive particel :["<<Momentum_Range_Plus[0]<<" , "<<Momentum_Range_Plus[1]<<")."<<endl; }
    return 0; 
  }
  
  if(Charge_Tem<0 && (Momentum_Mag/fabs(Charge_Tem)>Momentum_Range_Minus[1] || Momentum_Mag/fabs(Charge_Tem)<Momentum_Range_Minus[0])) 
  {
    if(IsDebug==1) { cout<<"Current momentum: "<<Momentum_Mag<<" Charge: "<<Charge_Tem<<", the range of negative particel :["<<Momentum_Range_Minus[0]<<" , "<<Momentum_Range_Minus[1]<<")."<<endl; }
    return 0; 
  }
  
  Momentum_Mag = fabs(Momentum_Mag/Charge_Tem);
  
  int ThetaIndex = (int) (Theta_Tem/ThetaBin_Unit);
  int PhiIndex = (int) ((Phi_Tem+180)/PhiBin_Unit);
//  cout<<"ThetaIndex: "<<ThetaIndex<<"  PhiIndex:  "<<PhiIndex<<endl;
  
  if(Charge_Tem>0)
  {
    for(int i=0;i<MomentumNum_Plus;i++)
    {
      if(Momentum_Mag==MomentumValue_Plus[i]) { return (ClusterNum_AlongX_Plus[ThetaIndex][PhiIndex][i]+ClusterNum_AlongZ_Plus[ThetaIndex][PhiIndex][i]); }
      else if(Momentum_Mag>MomentumValue_Plus[i] && i+1<MomentumNum_Plus && Momentum_Mag<MomentumValue_Plus[i+1]) 
      {// the linear interpolation
        double k = (Momentum_Mag-MomentumValue_Plus[i])/(MomentumValue_Plus[i+1]-Momentum_Mag);
        double n1 = (ClusterNum_AlongX_Plus[ThetaIndex][PhiIndex][i]+ClusterNum_AlongZ_Plus[ThetaIndex][PhiIndex][i]);
        double n2 = (ClusterNum_AlongX_Plus[ThetaIndex][PhiIndex][i+1]+ClusterNum_AlongZ_Plus[ThetaIndex][PhiIndex][i+1]);
        return ((k*n2+n1)/(1+k));
      }
    }
    cout<<Momentum_Mag<<" MeV/c is not in the range of DB"<<endl; 
    return 0;
  }
  else if(Charge_Tem<0)
  {
    for(int i=0;i<MomentumNum_Minus;i++)
    {
      if(Momentum_Mag==MomentumValue_Minus[i]) { return (ClusterNum_AlongX_Minus[ThetaIndex][PhiIndex][i]+ClusterNum_AlongZ_Minus[ThetaIndex][PhiIndex][i]); }
      else if(Momentum_Mag>MomentumValue_Minus[i] && i+1<MomentumNum_Minus && Momentum_Mag<MomentumValue_Minus[i+1]) 
      {// the linear interpolation
        double k = (Momentum_Mag-MomentumValue_Minus[i])/(MomentumValue_Minus[i+1]-Momentum_Mag);
        double n1 = (ClusterNum_AlongX_Minus[ThetaIndex][PhiIndex][i]+ClusterNum_AlongZ_Minus[ThetaIndex][PhiIndex][i]);
        double n2 = (ClusterNum_AlongX_Minus[ThetaIndex][PhiIndex][i+1]+ClusterNum_AlongZ_Minus[ThetaIndex][PhiIndex][i+1]);
        return ((k*n2+n1)/(1+k));
      }
    }
    cout<<Momentum_Mag<<" MeV/c is not in the range of DB"<<endl; 
    return 0;
  }
  else
  {
     if(IsDebug==1) { cout<<"Charge "<<Charge_Tem<<" Not Existed in the DB!"<<endl; return 0; }
  }
return 0;
}

//get all the information of DataBase
//double* DB_Results: ClusterNum_AlongX, ClusterNum_AlongZ,  TrackLength_XZ, TrackLength_3D, dEdX_Avg, dEdX_Truncated.
bool ST_ClusterNum_DB::Get_DB_Info(int Charge_Tem, TVector3 Momentum_Tem, double* DB_Results) 
{
  double Momentum_Mag = Momentum_Tem.Mag();
  double Theta_Tem = Momentum_Tem.Theta()*180.0/Pi();
  double Phi_Tem = Momentum_Tem.Phi()*180.0/Pi(); //here the Phi_tem belong to (-180,180)
  
  if(Theta_Tem>90) //90 degree
  {
    if(IsDebug==1)
    {
      cout<<"Theta > 90 degree"<<endl;
      cout<<Momentum_Tem.X()<<"  "<<Momentum_Tem.Y()<<"  "<<Momentum_Tem.Z()<<endl;
    }
    return 0;
  }
//  if(IsDebug==1)  { cout<<Momentum_Mag<<"  "<<Theta_Tem<<"  "<<Phi_Tem<<endl; }
  //the below is for checking the momentum range.
  if(Charge_Tem>0 && (Momentum_Mag/Charge_Tem>Momentum_Range_Plus[1] || Momentum_Mag/Charge_Tem<Momentum_Range_Plus[0])) 
  {
    if(IsDebug==1) { cout<<"Current momentum: "<<Momentum_Mag<<" Charge: "<<Charge_Tem<<", the range of positive particel :["<<Momentum_Range_Plus[0]<<" , "<<Momentum_Range_Plus[1]<<")."<<endl; }
    return 0; 
  }
  
  if(Charge_Tem<0 && (Momentum_Mag/fabs(Charge_Tem)>Momentum_Range_Minus[1] || Momentum_Mag/fabs(Charge_Tem)<Momentum_Range_Minus[0])) 
  {
    if(IsDebug==1) { cout<<"Current momentum: "<<Momentum_Mag<<" Charge: "<<Charge_Tem<<", the range of negative particel :["<<Momentum_Range_Minus[0]<<" , "<<Momentum_Range_Minus[1]<<")."<<endl; }
    return 0; 
  }
  
  Momentum_Mag = fabs(Momentum_Mag/Charge_Tem);
  
  int ThetaIndex = (int) (Theta_Tem/ThetaBin_Unit);
  int PhiIndex = (int) ((Phi_Tem+180)/PhiBin_Unit);
//  cout<<"ThetaIndex: "<<ThetaIndex<<"  PhiIndex:  "<<PhiIndex<<endl;
  
  if(Charge_Tem>0)
  {
    for(int i=0;i<MomentumNum_Plus;i++)
    {
      if(fabs(Momentum_Mag-MomentumValue_Plus[i])/MomentumValue_Plus[i]<0.01)
      {
        DB_Results[0] = ClusterNum_AlongX_Plus[ThetaIndex][PhiIndex][i];
        DB_Results[1] = ClusterNum_AlongZ_Plus[ThetaIndex][PhiIndex][i];
        DB_Results[2] = TrackLength_XZ_Plus[ThetaIndex][PhiIndex][i];
        DB_Results[3] = TrackLength_3D_Plus[ThetaIndex][PhiIndex][i];
        DB_Results[4] = dEdX_Avg_Plus[ThetaIndex][PhiIndex][i];
        DB_Results[5] = dEdX_Truncated_Plus[ThetaIndex][PhiIndex][i];
        return 1;
      }
      else if(Momentum_Mag>MomentumValue_Plus[i] && i+1<MomentumNum_Plus && Momentum_Mag<MomentumValue_Plus[i+1]) 
      {// the linear interpolation
        double k = (Momentum_Mag-MomentumValue_Plus[i])/(MomentumValue_Plus[i+1]-Momentum_Mag);
        double ClusterNum_AlongX_Plus_n1 = ClusterNum_AlongX_Plus[ThetaIndex][PhiIndex][i];
        double ClusterNum_AlongX_Plus_n2 = ClusterNum_AlongX_Plus[ThetaIndex][PhiIndex][i+1];
        double ClusterNum_AlongZ_Plus_n1 = ClusterNum_AlongZ_Plus[ThetaIndex][PhiIndex][i];
        double ClusterNum_AlongZ_Plus_n2 = ClusterNum_AlongZ_Plus[ThetaIndex][PhiIndex][i+1];
        double TrackLength_XZ_Plus_n1 = TrackLength_XZ_Plus[ThetaIndex][PhiIndex][i];
        double TrackLength_XZ_Plus_n2 = TrackLength_XZ_Plus[ThetaIndex][PhiIndex][i+1];
        double TrackLength_3D_Plus_n1 = TrackLength_3D_Plus[ThetaIndex][PhiIndex][i];
        double TrackLength_3D_Plus_n2 = TrackLength_3D_Plus[ThetaIndex][PhiIndex][i+1];
        double dEdX_Avg_Plus_n1 = dEdX_Avg_Plus[ThetaIndex][PhiIndex][i];
        double dEdX_Avg_Plus_n2 = dEdX_Avg_Plus[ThetaIndex][PhiIndex][i+1];
        double dEdX_Truncated_Plus_n1 = dEdX_Truncated_Plus[ThetaIndex][PhiIndex][i];
        double dEdX_Truncated_Plus_n2 = dEdX_Truncated_Plus[ThetaIndex][PhiIndex][i+1];
        
        DB_Results[0] = ((k*ClusterNum_AlongX_Plus_n2+ClusterNum_AlongX_Plus_n1)/(1+k));
        DB_Results[1] = ((k*ClusterNum_AlongZ_Plus_n2+ClusterNum_AlongZ_Plus_n1)/(1+k));
        DB_Results[2] = ((k*TrackLength_XZ_Plus_n2+TrackLength_XZ_Plus_n1)/(1+k));
        DB_Results[3] = ((k*TrackLength_3D_Plus_n2+TrackLength_3D_Plus_n1)/(1+k));
        DB_Results[4] = ((k*dEdX_Avg_Plus_n2+dEdX_Avg_Plus_n1)/(1+k));
        DB_Results[5] = ((k*dEdX_Truncated_Plus_n2+dEdX_Truncated_Plus_n1)/(1+k));
        return 1;
      }
    }
    if(IsDebug==1) { cout<<Momentum_Mag<<" MeV/c is not in the range of DB"<<endl; }
    return 0;
  }
  else if(Charge_Tem<0)
  {
    for(int i=0;i<MomentumNum_Minus;i++)
    {
      if(fabs(Momentum_Mag-MomentumValue_Minus[i])/MomentumValue_Minus[i]<0.01) 
      { 
        DB_Results[0] = ClusterNum_AlongX_Minus[ThetaIndex][PhiIndex][i];
        DB_Results[1] = ClusterNum_AlongZ_Minus[ThetaIndex][PhiIndex][i];
        DB_Results[2] = TrackLength_XZ_Minus[ThetaIndex][PhiIndex][i];
        DB_Results[3] = TrackLength_3D_Minus[ThetaIndex][PhiIndex][i];
        DB_Results[4] = dEdX_Avg_Minus[ThetaIndex][PhiIndex][i];
        DB_Results[5] = dEdX_Truncated_Minus[ThetaIndex][PhiIndex][i];
        return 1; 
      }
      else if(Momentum_Mag>MomentumValue_Minus[i] && i+1<MomentumNum_Minus && Momentum_Mag<MomentumValue_Minus[i+1]) 
      {// the linear interpolation
        double k = (Momentum_Mag-MomentumValue_Minus[i])/(MomentumValue_Minus[i+1]-Momentum_Mag);
        double ClusterNum_AlongX_Minus_n1 = ClusterNum_AlongX_Minus[ThetaIndex][PhiIndex][i];
        double ClusterNum_AlongX_Minus_n2 = ClusterNum_AlongX_Minus[ThetaIndex][PhiIndex][i+1];
        double ClusterNum_AlongZ_Minus_n1 = ClusterNum_AlongZ_Minus[ThetaIndex][PhiIndex][i];
        double ClusterNum_AlongZ_Minus_n2 = ClusterNum_AlongZ_Minus[ThetaIndex][PhiIndex][i+1];
        double TrackLength_XZ_Minus_n1 = TrackLength_XZ_Minus[ThetaIndex][PhiIndex][i];
        double TrackLength_XZ_Minus_n2 = TrackLength_XZ_Minus[ThetaIndex][PhiIndex][i+1];
        double TrackLength_3D_Minus_n1 = TrackLength_3D_Minus[ThetaIndex][PhiIndex][i];
        double TrackLength_3D_Minus_n2 = TrackLength_3D_Minus[ThetaIndex][PhiIndex][i+1];
        double dEdX_Avg_Minus_n1 = dEdX_Avg_Minus[ThetaIndex][PhiIndex][i];
        double dEdX_Avg_Minus_n2 = dEdX_Avg_Minus[ThetaIndex][PhiIndex][i+1];
        double dEdX_Truncated_Minus_n1 = dEdX_Truncated_Minus[ThetaIndex][PhiIndex][i];
        double dEdX_Truncated_Minus_n2 = dEdX_Truncated_Minus[ThetaIndex][PhiIndex][i+1];
        
        DB_Results[0] = ((k*ClusterNum_AlongX_Minus_n2+ClusterNum_AlongX_Minus_n1)/(1+k));
        DB_Results[1] = ((k*ClusterNum_AlongZ_Minus_n2+ClusterNum_AlongZ_Minus_n1)/(1+k));
        DB_Results[2] = ((k*TrackLength_XZ_Minus_n2+TrackLength_XZ_Minus_n1)/(1+k));
        DB_Results[3] = ((k*TrackLength_3D_Minus_n2+TrackLength_3D_Minus_n1)/(1+k));
        DB_Results[4] = ((k*dEdX_Avg_Minus_n2+dEdX_Avg_Minus_n1)/(1+k));
        DB_Results[5] = ((k*dEdX_Truncated_Minus_n2+dEdX_Truncated_Minus_n1)/(1+k));
        return 1;
      }
    }
    if(IsDebug==1) { cout<<Momentum_Mag<<" MeV/c is not in the range of DB"<<endl; }
    return 0;
  }
  else
  {
     if(IsDebug==1) { cout<<"Charge "<<Charge_Tem<<" Not Existed in the DB!"<<endl; return 0; }
  }
return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//the below functions are used to draw the data base.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//this function can be used to show the clusterNum on the (Theta, Phi)
void ST_ClusterNum_DB::Draw_Theta_Phi_Dis(int Charge_Tem, double Momentum_Tem)
{
  if(Charge_Tem>0 && (Momentum_Tem>Momentum_Range_Plus[1] || Momentum_Tem<Momentum_Range_Plus[0])) 
  { cout<<"The range of positive particel :["<<Momentum_Range_Plus[0]<<" , "<<Momentum_Range_Plus[1]<<")."<<endl; return; }
  if(Charge_Tem<0 && (Momentum_Tem>Momentum_Range_Minus[1] || Momentum_Tem<Momentum_Range_Minus[0])) 
  { cout<<"The range of negative particel :["<<Momentum_Range_Minus[0]<<" , "<<Momentum_Range_Minus[1]<<")."<<endl; return; }
  
  if( !(Charge_Tem==1 || Charge_Tem==-1) )
  { cout<<"Charge "<<Charge_Tem<<" Not Existed!"<<endl; return; }
  
  //for drawing
  char NameTem[200];
  sprintf(NameTem,"c1_ThetaPhi_Dis_%.2fMeV",Momentum_Tem);
  TCanvas* c1_ThetaPhi_Dis = new TCanvas(NameTem,NameTem,600,1200);
  c1_ThetaPhi_Dis->Divide(3,2);

  TH2D* h2_DB_Info[6];
  string DBInfo_Name[6] = {"ClusterNum_AlongX","ClusterNum_AlongZ","TrackLength_XZ","TrackLength_3D","dEdX_Avg","dEdX_Truncated"};
  for(int i=0;i<6;i++)
  {
    sprintf(NameTem,"h2_ThetaPhi_%s_Dis_Charge%d_%.2fMeV",DBInfo_Name[i].c_str(),Charge_Tem,Momentum_Tem);
    h2_DB_Info[i] = new TH2D(NameTem,";#theta(Deg.);#phi(Deg.)",THETANUM,0,90,PHINUM,-180,180);
  }

  for(int iTheta=0; iTheta<THETANUM;iTheta++)
  {
    for(int iPhi=0;iPhi<PHINUM;iPhi++)
    {
      double DB_Results[10] = {0};
      TVector3 Momentum_Vec_Tem(0,0,Momentum_Tem);
      Momentum_Vec_Tem.SetTheta((iTheta*ThetaBin_Unit)*Pi()/180.0);
      Momentum_Vec_Tem.SetPhi((iPhi*PhiBin_Unit-180)*Pi()/180.0);
      Momentum_Vec_Tem.SetMag(Momentum_Tem);
      Get_DB_Info(Charge_Tem, Momentum_Vec_Tem, DB_Results);
      for(int i=0;i<6;i++)
      {
        if(i==5 && (DB_Results[i]>800 || DB_Results[i]<0 )) {DB_Results[i] = 0;} 
        h2_DB_Info[i]->SetBinContent(iTheta+1,iPhi+1,DB_Results[i]);
      }
    }
  }
  
  for(int i=0;i<6;i++)
  {
    c1_ThetaPhi_Dis->cd(i+1); 
    h2_DB_Info[i]->Draw("colz");
  }
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
    if(Charge_Tem==1)
    {
      ClusterNum_Tem[iMomentum] = GetClusterNum(Charge_Tem,Theta_Tem,Phi_Tem,MomentumValue_Plus[iMomentum]);
      MomentumValue_Tem[iMomentum] = MomentumValue_Plus[iMomentum];
    }
    else if(Charge_Tem==-1)
    {
      ClusterNum_Tem[iMomentum] = GetClusterNum(Charge_Tem,Theta_Tem,Phi_Tem,MomentumValue_Minus[iMomentum]);
      MomentumValue_Tem[iMomentum] = MomentumValue_Minus[iMomentum];
    }
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
