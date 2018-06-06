//---------------------------------------------------------------------
// Description:
//  PSA will produce STHit, which will be used to construct the track. This class will focus on the performance of hit.
// 1, how many hit will be produced, and how many hits are on one pad, and what is the density of the hit in the TPC.
// 2, how many hit will be used to construct the track, and the relation between the track and the piled up hits.
// 3, what is the relation between the residual error of track with the hit density, rise edge fitting.
// Author List:
//      Rensheng     Soochow Univ.       (original author)
//----------------------------------------------------------------------

#ifndef _STAnaHit_H_
#define _STAnaHit_H_

#include "TCutG.h"
#include "TMath.h"
#include "iostream"
#include "fstream"
#include "TH1D.h"
#include "TH2D.h"
#include "TClonesArray.h"
#include "TCanvas.h"
#include "string"

#include "STRecoTrack.hh"
#include "STVertex.hh"
#include "STHit.hh"
#include "STRecoTrack.hh"
#include "STVertex.hh"
#include "STRawEvent.hh"
#include "STHelixTrack.hh"
#include "STHitCluster.hh"
#include "STPickTrackCut.hh"

using namespace TMath;
using namespace std;

class STAnaHit
{
public:
  // Constructor and Destructor
  STAnaHit();
  ~STAnaHit();
  void Initial(string tag);
  void Draw();
  ofstream outfile;
  
public:
// Get functions:
  void Ana_UsedHit(int EvtID);
  void Ana_HitDensity(int EvtID);
  
// Set functions:
  void SetAnaRowIndex(int Min,int Max) { MinRowIndex=Min; MaxRowIndex=Max; }
  void SetAnaLayerIndex(int Min,int Max) { MinLayerIndex=Min; MaxLayerIndex=Max;}
  void SetHitArray(TClonesArray* Array) { fHitArray = Array; }
  void SetHitClusterArray(TClonesArray* Array) { fHitClusterArray = Array; }
  void SetHelixTrackArray(TClonesArray* Array) { fHelixTrackArray = Array; }
  void SetVertexArray(TClonesArray* Array) { fVertexArray = Array; }
  
  void SetRecoTrackArray(TClonesArray* Array) { fRecoTrackArray = Array; }
  void Set_IsAna_HitDensity(bool IsTem) { IsAna_HitDensity = IsTem; }

public:
  //the hit num in one event, and the used num.
  TH1D* h1_HitNum_1Pad;
  TH1D* h1_UsedHitNum_1Pad;
  TCanvas* c1_HitNum_1Pad;
  
  //the hit density, and the relation with the used ratio
  double TotalHitNum_1Pad[50];
  double TotalUsedHitNum_1Pad[50];
  double TotalHitNum_30Tb[50];
  double TotalUsedHitNum_30Tb[50];
  double TotalHitNum_20Tb[50];
  double TotalUsedHitNum_20Tb[50];
  double TotalHitNum_10Tb[50];
  double TotalUsedHitNum_10Tb[50];
  TH1D* h1_Hit_UsedRatio_Total;
  TH1D* h1_Hit_UsedRatio_1Pad;
  TH1D* h1_Hit_UsedRatio_HitDensity_30Tb;
  TH1D* h1_Hit_UsedRatio_HitDensity_20Tb;
  TH1D* h1_Hit_UsedRatio_HitDensity_10Tb;
  TCanvas* c1_Hit_UsedRatio;
  
  //the hit density distribution in the TPC.
  TH2D* h2_MultiHit_PadPlane;
  TH2D* h2_MultiHit_SideView;
  TCanvas* c1_MultiHit_Dis;
  int AnaEvtNum;
  
  //the hit density and the residual error of track.
  TH1D* h1_TrackResErr;
  TH2D* h2_TrackResErr_HitDensity;
  TCanvas* c1_TrackResErr;
  
  //the saturated pulse
  TH2D* h2_Saturated_FollowHit;
  TCanvas* c1_Saturated_FollowHit;
  int SaturatedPadNum;
  
private:
  bool IsAna_HitDensity;
  int MinRowIndex; int MaxRowIndex;
  int MinLayerIndex; int MaxLayerIndex;
  TClonesArray* fHitArray;
  TClonesArray* fHitClusterArray;
  TClonesArray* fHelixTrackArray;
  TClonesArray* fRecoTrackArray;
  TClonesArray* fVertexArray;
  
  STPickTrackCut* aTrackCut;
  
  ClassDef(STAnaHit, 1);
};

#endif
