#include "ST_ProduceDB_ClusterNum.hh"
ClassImp(ST_ProduceDB_ClusterNum);

ST_ProduceDB_ClusterNum::ST_ProduceDB_ClusterNum()
{
  
}


ST_ProduceDB_ClusterNum::~ST_ProduceDB_ClusterNum()
{;}

void ST_ProduceDB_ClusterNum::Initial(string NameTem)
{
  //the below is for setting the value.
  PositiveMomentumNum = 78;
  string PosMomentumTem[] = {"50","52.5","55","57.5","60","62.5","65","67.5","70","72.5","75","77.5","80","82.5","85","87.5","90","92.5","95","97.5",
                          "100","110","120","130","140","150","160","170","180","190",
                          "200","220","240","260","280",
                          "300","325","350","375","400","425","450","475",
                          "500","550","600","650","700","750","800","850","900","950",
                          "1000","1100","1200","1300","1400","1500","1600","1700","1800","1900","2000","2100","2200","2300","2400","2500","2600","2700","2800","2900",
                          "3000","3250","3500","3750","4000"};

  NegativeMomentumNum = 54;
  string NegMomentumTem[] = {"50","52.5","55","57.5","60","62.5","65","67.5","70","72.5","75","77.5","80","82.5","85","87.5","90","92.5","95","97.5",
                          "100","110","120","130","140","150","160","170","180","190",
                          "200","220","240","260","280",
                          "300","325","350","375","400","425","450","475",
                          "500","550","600","650","700","750","800","850","900","950","1000"};
  
  MomentumConfigFileName = NameTem;
  ofstream MomentumConfigFile(MomentumConfigFileName.c_str());
  
  //the below is for positive charged particles.
  MomentumConfigFile<<endl<<"# plus"<<endl;
  for(int i=0;i<PositiveMomentumNum;i++) 
  {
    PositiveMomentumValue[i] = PosMomentumTem[i];
    MomentumConfigFile<<PositiveMomentumValue[i]<<" 1 "<<endl;
  }
  MomentumConfigFile<<endl<<"# minus"<<endl;
  for(int i=0;i<NegativeMomentumNum;i++) 
  {
    NegativeMomentumValue[i] = NegMomentumTem[i];
    MomentumConfigFile<<NegativeMomentumValue[i]<<" -1 "<<endl;
  }
}

void ST_ProduceDB_ClusterNum::BuildDB(string f1_DB_FileName)
{
  TFile* f1_DB = new TFile(f1_DB_FileName.c_str(),"recreate");
  
  string TotalEvtNumTag = "2E5";
  char NameTem[200];
  //the below is for positive particles.
  for(int i=0;i<PositiveMomentumNum;i++)
//  for(int i=0;i<1;i++)
  {
    sprintf(NameTem,"./Data/mu+_%s_%s.root",PositiveMomentumValue[i].c_str(),TotalEvtNumTag.c_str());
    ReadRawData(1,atof(PositiveMomentumValue[i].c_str()),NameTem,f1_DB);
  }

  //the below is for negative particles.
  for(int i=0;i<NegativeMomentumNum;i++)
  {
    sprintf(NameTem,"./Data/mu-_%s_%s.root",PositiveMomentumValue[i].c_str(),TotalEvtNumTag.c_str());
    ReadRawData(-1,atof(NegativeMomentumValue[i].c_str()),NameTem,f1_DB);
  }

  f1_DB->Close();
}

void ST_ProduceDB_ClusterNum::ReadRawData(int ChargeValue,double MomentumValue,string RawDataFileName,TFile* f1_DB_ClusterNum)
{
  if(f1_DB_ClusterNum!=0) { f1_DB_ClusterNum->cd(); }
  else { cout<<"f1_DB_ClusterNum = null pointer!"<<endl; return; }
  
  cout<<"Deal with : "<<RawDataFileName<<" ... "<<endl;
  
  char NameTem[200];
  sprintf(NameTem,"t1_DB_ClusterNum_Charge%d_Momentum%.2f",ChargeValue,MomentumValue);
  TTree* t1_DB_ClusterNum = new TTree(NameTem,"DB_Cluster_Number");
  
  const int ThetaNum = 90;
  const int PhiNum = 180;
  double ThetaBin_Unit_G4 = 90.0/ThetaNum;
  double PhiBin_Unit_G4 = 360/PhiNum;
  
  int Charge_DB = ChargeValue;
  double Momentum_DB = MomentumValue;
  double ClusterNum_AlongX[ThetaNum][PhiNum] = {{0}};
  double ClusterNum_AlongZ[ThetaNum][PhiNum] = {{0}};
  double TrackLength_XZ[ThetaNum][PhiNum] = {{0}};
  double TrackLength_3D[ThetaNum][PhiNum] = {{0}};
  double dEdX_Avg[ThetaNum][PhiNum] = {{0}};
  double dEdX_Truncated[ThetaNum][PhiNum] = {{0}};
  
  t1_DB_ClusterNum->Branch("Charge",&Charge_DB,"Charge_DB/I");
  t1_DB_ClusterNum->Branch("Momentum",&Momentum_DB,"Momentum_DB/D");
  t1_DB_ClusterNum->Branch("ClusterNum_AlongX",ClusterNum_AlongX,"ClusterNum_AlongX[90][180]/D");
  t1_DB_ClusterNum->Branch("ClusterNum_AlongZ",ClusterNum_AlongZ,"ClusterNum_AlongZ[90][180]/D");
  t1_DB_ClusterNum->Branch("TrackLength_XZ",TrackLength_XZ,"TrackLength_XZ[90][180]/D");
  t1_DB_ClusterNum->Branch("TrackLength_3D",TrackLength_3D,"TrackLength_3D[90][180]/D");
  t1_DB_ClusterNum->Branch("dEdX_Avg",dEdX_Avg,"dEdX_Avg[90][180]/D");
  t1_DB_ClusterNum->Branch("dEdX_Truncated",dEdX_Truncated,"dEdX_Truncated[90][180]/D");
  
  //the below is for reading the data from geant simulation results.
  TFile* f1_SimData_G4 = new TFile(RawDataFileName.c_str());
  TTree* t1_SimData_G4 = (TTree*)f1_SimData_G4->Get("t1");
    
  double Theta_G4;
  double Phi_G4;
  int ClusterNum_AlongX_G4;
  int ClusterNum_AlongZ_G4;
  double TrackLength_XZ_G4;
  double TrackLength_3D_G4;
  double dEdX_Avg_G4;
  double dEdX_Truncated_G4;
  
  t1_SimData_G4->SetBranchAddress("Theta",&Theta_G4);
  t1_SimData_G4->SetBranchAddress("Phi",&Phi_G4);
  t1_SimData_G4->SetBranchAddress("ClusterNum_AlongX",&ClusterNum_AlongX_G4);
  t1_SimData_G4->SetBranchAddress("ClusterNum_AlongZ",&ClusterNum_AlongZ_G4);
  t1_SimData_G4->SetBranchAddress("TrackLength_3D",&TrackLength_3D_G4);
  t1_SimData_G4->SetBranchAddress("TrackLength_XZ",&TrackLength_XZ_G4);
  t1_SimData_G4->SetBranchAddress("dEdX_Avg",&dEdX_Avg_G4);
  t1_SimData_G4->SetBranchAddress("dEdX_Truncated",&dEdX_Truncated_G4);
  
  int EvtNum_G4 = t1_SimData_G4->GetEntries();
  cout<<"EvtNum_G4: "<<EvtNum_G4<<endl;
  
  double G4_EvtCount[ThetaNum][PhiNum] = {{0}};
  double ClusterNum_AlongX_G4_Value[ThetaNum][PhiNum] = {{0}};
  double ClusterNum_AlongZ_G4_Value[ThetaNum][PhiNum] = {{0}};
  double TrackLength_XZ_G4_Value[ThetaNum][PhiNum] = {{0}};
  double TrackLength_3D_G4_Value[ThetaNum][PhiNum] = {{0}};
  double dEdX_Avg_G4_Value[ThetaNum][PhiNum] = {{0}};
  double dEdX_Truncated_G4_Value[ThetaNum][PhiNum] = {{0}};
    
  for(int iEvent_G4=0;iEvent_G4<EvtNum_G4;iEvent_G4++)
  {
    t1_SimData_G4->GetEntry(iEvent_G4);
    int ThetaIndex = (int) Theta_G4/ThetaBin_Unit_G4;
    int PhiIndex = (int) (Phi_G4+180)/PhiBin_Unit_G4;
    
    G4_EvtCount[ThetaIndex][PhiIndex] += 1;
    ClusterNum_AlongX_G4_Value[ThetaIndex][PhiIndex] += ClusterNum_AlongX_G4;
    ClusterNum_AlongZ_G4_Value[ThetaIndex][PhiIndex] += ClusterNum_AlongZ_G4;
    TrackLength_XZ_G4_Value[ThetaIndex][PhiIndex] += TrackLength_XZ_G4;
    TrackLength_3D_G4_Value[ThetaIndex][PhiIndex] += TrackLength_3D_G4;
    dEdX_Avg_G4_Value[ThetaIndex][PhiIndex] += dEdX_Avg_G4;
    dEdX_Truncated_G4_Value[ThetaIndex][PhiIndex] += dEdX_Truncated_G4;
  }
  
  for(int iTheta=0;iTheta<ThetaNum;iTheta++)
  {
    for(int iPhi=0;iPhi<PhiNum;iPhi++)
    {
      if(G4_EvtCount[iTheta][iPhi]!=0) 
      {
        ClusterNum_AlongX[iTheta][iPhi] = ClusterNum_AlongX_G4_Value[iTheta][iPhi]/G4_EvtCount[iTheta][iPhi]; //get the average value.
        ClusterNum_AlongZ[iTheta][iPhi] = ClusterNum_AlongZ_G4_Value[iTheta][iPhi]/G4_EvtCount[iTheta][iPhi]; //get the average value.
        TrackLength_XZ[iTheta][iPhi] = TrackLength_XZ_G4_Value[iTheta][iPhi]/G4_EvtCount[iTheta][iPhi]; //get the average value.
        TrackLength_3D[iTheta][iPhi] = TrackLength_3D_G4_Value[iTheta][iPhi]/G4_EvtCount[iTheta][iPhi]; //get the average value.
        dEdX_Avg[iTheta][iPhi] = dEdX_Avg_G4_Value[iTheta][iPhi]/G4_EvtCount[iTheta][iPhi]; //get the average value.
        dEdX_Truncated[iTheta][iPhi] = dEdX_Truncated_G4_Value[iTheta][iPhi]/G4_EvtCount[iTheta][iPhi]; //get the average value.
      }
      else 
      { 
        ClusterNum_AlongX[iTheta][iPhi] = 0; 
        ClusterNum_AlongZ[iTheta][iPhi] = 0;
        TrackLength_XZ[iTheta][iPhi] = 0;
        TrackLength_3D[iTheta][iPhi] = 0;
        dEdX_Avg[iTheta][iPhi] = 0;
        dEdX_Truncated[iTheta][iPhi] = 0;
      }
    }
  }
    
  //the below is for storing the DB tree.
  f1_DB_ClusterNum->cd();
  t1_DB_ClusterNum->Fill();
  t1_DB_ClusterNum->Write("",TObject::kOverwrite);
  t1_DB_ClusterNum->Delete();
}
