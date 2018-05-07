//---------------------------------------------------------------------
// Description:
//  This cut can be used to judge is this track meet our needs.
//
// Author List:
//      Rensheng     Soochow Univ.       (original author)
//----------------------------------------------------------------------

#ifndef _STPickTrackCut_H_
#define _STPickTrackCut_H_

#include "TCutG.h"
#include "TMath.h"
#include "iostream"
#include "STRecoTrack.hh"
#include "STVertex.hh"

using namespace TMath;
using namespace std;

class STPickTrackCut
{
public:
  // Constructor and Destructor
  STPickTrackCut();
  ~STPickTrackCut();
    
  // Operators
  // Getters
  TCutG* GetPionPlusCut() { return PionPlus_PIDCut; }
  TCutG* GetPionMinusCut() { return PionMinus_PIDCut; }
  // Setters
  void SetParticlePID(int NumTem, int* PID_Index_tem, int* Charge_tem)
  { 
    ParticleNum = NumTem;
    for(int i=0;i<ParticleNum;i++)
    {
      PID_Index[i] = PID_Index_tem[i];
      charge[i] = Charge_tem[i];
    }
  }
  void Set_IsVertexLimited(bool IsLimited){ IsVertexLimited = IsLimited; }
  void Set_Track_DistanceToVertex(double distance_tem){ Track_DistanceToVertex_Cut = distance_tem; }
  void Set_Track_ClusterNum(int Num_tem) { Track_ClusterNum_Cut = Num_tem; }
  
  // Main methods
  bool IsInCut(int PID_Index_tem, int Charge_tem, STRecoTrack* aTrack,STVertex* aVertex);
  void Initial_PIDCut();
  bool IsInPionPlusCut(STRecoTrack* aTrack);
  bool IsInPionMinusCut(STRecoTrack* aTrack);
  bool IsTrackInVertex(STRecoTrack* aTrack,STVertex* aVertex);
  bool IsClusterNumEnough(STRecoTrack* aTrack);
  bool IsVertexInTarget(STVertex* aVertex);
  
  private:
  bool IsVertexLimited;
  //these Target Position information is fixed in the code, now.
  double TargetPosX[2];
  double TargetPosY[2];
  double TargetPosZ;
  double TargetPosZ_Sigma;
  
  //some cut on track
  double Track_DistanceToVertex_Cut;
  int Track_ClusterNum_Cut;
  
  // the PID that you want to filter out!
  int ParticleNum;
  int PID_Index[10];
  int charge[10];
  
  //the below is for the single particle PID, these cut is made by hand.
  double PionPlus_PIDCut_Value[100][2];
  int PionPlus_PIDCut_PointNum;
  TCutG* PionPlus_PIDCut;
  
  double PionMinus_PIDCut_Value[100][2];
  int PionMinus_PIDCut_PointNum;
  TCutG* PionMinus_PIDCut;
  
  ClassDef(STPickTrackCut, 1);
};

#endif
