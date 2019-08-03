/*
  ST_VertexShift is used to get the BDC correction database according to the difference between TPC-Vertex on the Target( The BDC's center is same with TPC-Vertex on both X and Y direction ) and the target postion of STRecoTrack.
1, Normally, we will divide the phase space with this way: 
        Phi: 0-360, each 10 deg will be as a unit.
        Theta: 0-90, each 1 deg will be as a unit.

2, First you need some events to produce the correlation (TH2D) of between TPC-Vertex on the Target and the target postion of STRecoTrack in a root file( "f1_BDCCorrection_Theta_TargetPos.root" ) with ST_VertexShift::Cal_BDC_Correction(). In this function, the mean profile and mean histogram will be produced, and stored in the same file.

3, If you can make sure there are already mean histogram is already in the root file, then you can use the ST_VertexShift::Load_BDC_Correction() to load the mean profile and mean histogram directly.

4, Normally, you should use ST_VertexShift::GetShiftValue_Smooth() to get the shift value.
*/
#ifndef ST_VertexShift_h
#define ST_VertexShift_h 1

#include "fstream"
#include "sstream"
#include "string.h"
#include "iostream"

#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

#include "TMath.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TVector3.h"
#include "TCutG.h"

#define MaxPhiNum_ST_VertexShift    100

using namespace std;
using namespace TMath;

class ST_VertexShift : public TObject
{
public:
  ST_VertexShift();
  ~ST_VertexShift();
  
public:
  int PhiNum;
  TH2D* h2_Theta_TargetPosX[MaxPhiNum_ST_VertexShift] = {0};
  TH2D* h2_Theta_TargetPosY[MaxPhiNum_ST_VertexShift] = {0};
  
  TProfile* Profile_Theta_TargetPosX[MaxPhiNum_ST_VertexShift] = {0};
  TProfile* Profile_Theta_TargetPosY[MaxPhiNum_ST_VertexShift] = {0};
  
  //when I use the smooth(), the profile always show some strange characters, so I use the TH1D to replace the TProfile.
  TH1D* h1_Theta_TargetPosX_Smooth[MaxPhiNum_ST_VertexShift] = {0}; 
  TH1D* h1_Theta_TargetPosY_Smooth[MaxPhiNum_ST_VertexShift] = {0};
  
  void GetShiftValue(double Theta,double Phi,double* ShiftX,double* ShiftY);
  void GetShiftValue_Smooth(double Theta,double Phi,double* ShiftX,double* ShiftY);
  double Get_Corrected_DistToVertex_OnTarget(double Theta, double Phi, TVector3 PosOnTarget, TVector3 PosVertexOnTarget);
  
  TProfile* GetProfileX(int Index){ return Profile_Theta_TargetPosX[Index]; }
  TH1D* GetProfileX_Smooth(int Index){ return h1_Theta_TargetPosX_Smooth[Index]; }
  TProfile* GetProfileY(int Index){ return Profile_Theta_TargetPosY[Index]; }
  TH1D* GetProfileY_Smooth(int Index){ return h1_Theta_TargetPosY_Smooth[Index]; }
  TH2D* Get_Theta_TargetPosX(int Index) { return h2_Theta_TargetPosX[Index]; }
  TH2D* Get_Theta_TargetPosY(int Index) { return h2_Theta_TargetPosY[Index]; }
  
  void Cal_BDC_Correction(string FileName); //from TH2D to profile and TH1D( with smooth ).
  void Load_BDC_Correction(string FileName); // load TH2D, profile, and TH1D if the file has.
  void Clean_Count(TH2D* h2_tem);
  
  int Threshold_Remove_BG; //normally, it will be applied to the whole histogram. 
  int Opt_Remove_BG; //0: remove the count for the whole histogram; 
                     //1: remove the different count for the  different theta region.
  double ThetaCountClear_Fraction; //this value is belong to (0,1).
  
  void Set_Threshold_Remove_BG(int tem) { Threshold_Remove_BG = tem; }
  void Set_Opt_Remove_BG(int tem) { Opt_Remove_BG = tem; }
  void Set_ThetaCountClear_Fraction(double tem);

  ClassDef(ST_VertexShift,1)
};

#endif
