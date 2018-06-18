#ifndef ST_TrackCut_h
#define ST_TrackCut_h 1

#include "fstream"
#include "sstream"
#include "string.h"
#include "iostream"

#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"

#include "TMath.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TVector3.h"
#include "TCutG.h"

#include "STVertex.hh"
#include "STRecoTrack.hh"

using namespace std;
using namespace TMath;

#define MaxCutNum	100

class ST_TrackCut : public TObject
{
public:
  ST_TrackCut();
  ~ST_TrackCut();
  
public:
  // the below is for setting and initialization.
  void Set_Track_DistanceToVertex(double distance_tem){ Track_DistanceToVertex_Cut = distance_tem; }
  void ReadCutFile(string FileName_Tem);
  void DrawCut();
  void DrawCut(TCanvas* c1_tem);
  
public:
  //the below is for get
  bool IsVertexInTarget(STVertex* aVertex);
  bool IsTrackInVertex(STRecoTrack* aTrack,STVertex* aVertex);
  TCutG* GetPIDCut(string CutTag);

private:
  //these Target Position information is fixed in the code, now.
  double TargetPosX[2];
  double TargetPosY[2];
  double TargetPosZ;
  double TargetPosZ_Sigma;
  
  //some cut on track
  double Track_DistanceToVertex_Cut;
  
  //The TCutG number in the cut file.
  int PIDCut_Num;
  string PIDCut_Tag[MaxCutNum];
  TCutG* PIDCut_Array[MaxCutNum];
  
  ClassDef(ST_TrackCut,1)
};

#endif
