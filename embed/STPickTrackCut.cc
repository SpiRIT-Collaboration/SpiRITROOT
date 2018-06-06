//---------------------------------------------------------------------
// Description:
// This class is used to judge whether the track should be stored.
//
// Author List:
//      Rensheng     Soochow Univ.
//
//----------------------------------------------------------------------

#include "STPickTrackCut.hh"

ClassImp(STPickTrackCut)

STPickTrackCut::STPickTrackCut()
{
  IsVertexLimited = 1;
  TargetPosX[0] = -15; TargetPosX[1] = 15;
  TargetPosY[0] = -246.06; TargetPosY[1] = -206.06;
  TargetPosZ = -11.9084;
  TargetPosZ_Sigma = 1.69675;
  
  Track_DistanceToVertex_Cut = 5; // unit: mm
  Track_ClusterNum_Cut = 30; 
  ParticleNum = 1;
  //it is only for initialize the PID cut for different particle.
  Initial_PIDCut();
}


STPickTrackCut::~STPickTrackCut()
{
  
}

bool STPickTrackCut::IsInCut(int PID_Index_tem, int Charge_tem, STRecoTrack* aTrack,STVertex* aVertex)
{
  //judge the vertex: is in the target
  if( IsVertexInTarget(aVertex)==0 ) {return 0;}
  //judge the track in the vertex
  if( IsTrackInVertex(aTrack,aVertex)==0 ) { return 0; }
  //judge the cluster num of one track.
  if( IsClusterNumEnough(aTrack)==0 ) { return 0; }
  //judge the PID
  if(PID_Index_tem==0 && Charge_tem==-1) { if( IsInPionMinusCut(aTrack)==0 ) return 0; }
  else if(PID_Index_tem==0 && Charge_tem==1)  { if( IsInPionPlusCut(aTrack)==0 ) return 0; }
  else 
  {
    cout<<"PID: "<<PID_Index_tem<<"  Charge: "<<Charge_tem<<"is not included in this function"<<endl;
    return 0;
  }
  
return 1;
}

bool STPickTrackCut::IsVertexInTarget(STVertex* aVertex)
{
  double Vertex_PosX = aVertex->GetPos().X();
  double Vertex_PosY = aVertex->GetPos().Y();
  double Vertex_PosZ = aVertex->GetPos().Z();
  bool TagTem = 0;
  if(Vertex_PosZ>TargetPosZ-3*TargetPosZ_Sigma && Vertex_PosZ<TargetPosZ+3*TargetPosZ_Sigma
   && Vertex_PosX> TargetPosX[0] && Vertex_PosX< TargetPosX[1]
   && Vertex_PosY> TargetPosY[0]&& Vertex_PosY< TargetPosY[1] )
  { TagTem = 1; }
  
return TagTem;
}

bool STPickTrackCut::IsClusterNumEnough(STRecoTrack* aTrack)
{
  vector<Int_t>* fHitClusterIDArray = aTrack->GetClusterIDArray();
  int ClusterNum = (*fHitClusterIDArray).size();
  bool TagTem = 0;
  if( ClusterNum>Track_ClusterNum_Cut ) { TagTem = 1; }
return TagTem;
}

bool STPickTrackCut::IsTrackInVertex(STRecoTrack* aTrack,STVertex* aVertex)
{
  double dist = (aTrack->GetPOCAVertex()-aVertex->GetPos()).Mag();
  bool TagTem = 0;
  if( dist<Track_DistanceToVertex_Cut ) { TagTem = 1; }
return TagTem;
}

bool STPickTrackCut::IsInPionPlusCut(STRecoTrack* aTrack)
{
  double Momentum = aTrack -> GetMomentum().Mag();
  auto dedx = aTrack -> GetdEdxWithCut(0,0.7);
  double Charge = aTrack->GetCharge();
  
  bool TagTem = 0;
  TagTem = PionPlus_PIDCut->IsInside(Momentum/Charge,dedx);
return TagTem;
}

bool STPickTrackCut::IsInPionMinusCut(STRecoTrack* aTrack)
{
  double Momentum = aTrack -> GetMomentum().Mag();
  auto dedx = aTrack -> GetdEdxWithCut(0,0.7);
  double Charge = aTrack->GetCharge();
  
  bool TagTem = 0;
  TagTem = PionMinus_PIDCut->IsInside(Momentum/Charge,dedx);
return TagTem;
}

bool STPickTrackCut::IsInElectronCut(STRecoTrack* aTrack)
{
  double Momentum = aTrack -> GetMomentum().Mag();
  auto dedx = aTrack -> GetdEdxWithCut(0,0.7);
  double Charge = aTrack->GetCharge();
  
  bool TagTem = 0;
  TagTem = Electron_PIDCut->IsInside(Momentum/Charge,dedx);
return TagTem;
}

bool STPickTrackCut::IsInPositronCut(STRecoTrack* aTrack)
{
  double Momentum = aTrack -> GetMomentum().Mag();
  auto dedx = aTrack -> GetdEdxWithCut(0,0.7);
  double Charge = aTrack->GetCharge();
  
  bool TagTem = 0;
  TagTem = Positron_PIDCut->IsInside(Momentum/Charge,dedx);
return TagTem;
}

bool STPickTrackCut::IsBetween_Pion_Proton(STRecoTrack* aTrack)
{
  double Momentum = aTrack -> GetMomentum().Mag();
  auto dedx = aTrack -> GetdEdxWithCut(0,0.7);
  double Charge = aTrack->GetCharge();
  
  bool TagTem = 0;
  TagTem = BetweenCut_Pion_Proton->IsInside(Momentum/Charge,dedx);
return TagTem;
}

void STPickTrackCut::Initial_PIDCut()
{  
//the below is for setting pion plus.
  PionMinus_PIDCut_PointNum = 11;
  PionMinus_PIDCut_Value[0][0] = -59.58; PionMinus_PIDCut_Value[0][1] = 91.3;
  PionMinus_PIDCut_Value[1][0] = -130.76; PionMinus_PIDCut_Value[1][1] = 54.1;
  PionMinus_PIDCut_Value[2][0] = -263.2; PionMinus_PIDCut_Value[2][1] = 28.2;
  PionMinus_PIDCut_Value[3][0] = -394.7; PionMinus_PIDCut_Value[3][1] = 26.7;
  PionMinus_PIDCut_Value[4][0] = -390.8; PionMinus_PIDCut_Value[4][1] = 10.0;
  PionMinus_PIDCut_Value[5][0] = -177.2; PionMinus_PIDCut_Value[5][1] = 11.3;
  PionMinus_PIDCut_Value[6][0] = -117.9; PionMinus_PIDCut_Value[6][1] = 21.8;
  PionMinus_PIDCut_Value[7][0] = -48.7; PionMinus_PIDCut_Value[7][1] = 21.8;
  PionMinus_PIDCut_Value[8][0] = -42.8; PionMinus_PIDCut_Value[8][1] = 31.0;
  PionMinus_PIDCut_Value[9][0] = -98.1; PionMinus_PIDCut_Value[9][1] = 31.5;
  PionMinus_PIDCut_Value[10][0] = -48.7; PionMinus_PIDCut_Value[10][1] = 88.2;
  
  PionMinus_PIDCut = new TCutG("PionMinus_PIDCut",PionMinus_PIDCut_PointNum);
  for(int i=0;i<PionMinus_PIDCut_PointNum;i++) { PionMinus_PIDCut->SetPoint(i,PionMinus_PIDCut_Value[i][0],PionMinus_PIDCut_Value[i][1]); }
  
//the below is for setting pion minus.
  PionPlus_PIDCut_PointNum = 7;
  PionPlus_PIDCut_Value[0][0] = 90.4; PionPlus_PIDCut_Value[0][1] = 41.7;
  PionPlus_PIDCut_Value[1][0] = 32.0; PionPlus_PIDCut_Value[1][1] = 17.6;
  PionPlus_PIDCut_Value[2][0] = 47.7; PionPlus_PIDCut_Value[2][1] = 13.4;
  PionPlus_PIDCut_Value[3][0] = 544; PionPlus_PIDCut_Value[3][1] = 12.7;
  PionPlus_PIDCut_Value[4][0] = 515; PionPlus_PIDCut_Value[4][1] = 22.1;
  PionPlus_PIDCut_Value[5][0] = 188.0; PionPlus_PIDCut_Value[5][1] = 33.5;
  PionPlus_PIDCut_Value[6][0] = 121.8; PionPlus_PIDCut_Value[6][1] = 42.2;
  
  PionPlus_PIDCut = new TCutG("PionPlus_PIDCut",PionPlus_PIDCut_PointNum);
  for(int i=0;i<PionPlus_PIDCut_PointNum;i++) { PionPlus_PIDCut->SetPoint(i,PionPlus_PIDCut_Value[i][0],PionPlus_PIDCut_Value[i][1]); }
  
  //the below is for setting electron.
  Electron_PIDCut_PointNum = 4;
  Electron_PIDCut_Value[0][0] = -121.3; Electron_PIDCut_Value[0][1] = 21.9;
  Electron_PIDCut_Value[1][0] = -49.6; Electron_PIDCut_Value[1][1] = 22.2;
  Electron_PIDCut_Value[2][0] = -45.2; Electron_PIDCut_Value[2][1] = 31.0;
  Electron_PIDCut_Value[3][0] = -99.4; Electron_PIDCut_Value[3][1] = 32.2;
  
  Electron_PIDCut = new TCutG("Electron_PIDCut",Electron_PIDCut_PointNum);
  for(int i=0;i<Electron_PIDCut_PointNum;i++) { Electron_PIDCut->SetPoint(i,Electron_PIDCut_Value[i][0],Electron_PIDCut_Value[i][1]); }
  
    //the below is for setting positron.
  Positron_PIDCut_PointNum = 5;
  Positron_PIDCut_Value[0][0] = 26.5; Positron_PIDCut_Value[0][1] = 27.7;
  Positron_PIDCut_Value[1][0] = 29.4; Positron_PIDCut_Value[1][1] = 18.1;
  Positron_PIDCut_Value[2][0] = 48.4; Positron_PIDCut_Value[2][1] = 14.1;
  Positron_PIDCut_Value[3][0] = 155.2; Positron_PIDCut_Value[3][1] = 13.1;
  Positron_PIDCut_Value[4][0] = 107.0; Positron_PIDCut_Value[4][1] = 26.3;
  
  Positron_PIDCut = new TCutG("Positron_PIDCut",Positron_PIDCut_PointNum);
  for(int i=0;i<Positron_PIDCut_PointNum;i++) { Positron_PIDCut->SetPoint(i,Positron_PIDCut_Value[i][0],Positron_PIDCut_Value[i][1]); }
  
  // the below is for setting Betweencut of pion and proton.
  BetweenCut_Pion_Proton_PointNum = 4;
  BetweenCut_Pion_Proton_Value[0][0] = 225.9; BetweenCut_Pion_Proton_Value[0][1] = 37;
  BetweenCut_Pion_Proton_Value[1][0] = 619.3; BetweenCut_Pion_Proton_Value[1][1] = 6;
  BetweenCut_Pion_Proton_Value[2][0] = 729.7; BetweenCut_Pion_Proton_Value[2][1] = 19;
  BetweenCut_Pion_Proton_Value[3][0] = 331.5; BetweenCut_Pion_Proton_Value[3][1] = 50;
  
  BetweenCut_Pion_Proton = new TCutG("BetweenCut_Pion_Proton",BetweenCut_Pion_Proton_PointNum);
  for(int i=0;i<BetweenCut_Pion_Proton_PointNum;i++) { BetweenCut_Pion_Proton->SetPoint(i,BetweenCut_Pion_Proton_Value[i][0],BetweenCut_Pion_Proton_Value[i][1]); }
}
