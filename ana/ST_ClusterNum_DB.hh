#ifndef ST_ClusterNum_DB_h
#define ST_ClusterNum_DB_h 1

#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "iostream"
#include "TMath.h"
#include "TF1.h"
#include "TCanvas.h"
#include "fstream"
#include "sstream"
#include "string.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TVector3.h"

using namespace std;
using namespace TMath;

#define	THETANUM	45
#define PHINUM		180
#define MaxMomentumNum_Plus	100
#define MaxMomentumNum_Minus	100

class ST_ClusterNum_DB : public TObject
{
public:
  ST_ClusterNum_DB();
  ~ST_ClusterNum_DB();
  
public:
  // the below is for setting and initialization.
  void Initial_Config(string FileName_Tem);
  void ReadDB(string FileName_Tem);
  void Set_MomentumRange_Plus(double* RangeTem) { Momentum_Range_Plus[0] = RangeTem[0]; Momentum_Range_Plus[1] = RangeTem[1]; }
  void Set_MomentumRange_Minus(double* RangeTem) { Momentum_Range_Minus[0] = RangeTem[0]; Momentum_Range_Minus[1] = RangeTem[1]; }
  
  // the below is for some functions.
  double GetClusterNum(int Charge_Tem, double Theta_Tem,double Phi_Tem,double Momentum_Tem);
  double GetClusterNum(int Charge_Tem, TVector3 Momentum_Tem);
  void Draw_Theta_Phi_Dis(int Charge_Tem, double Momentum_Tem,int Opt_interpolation);
  void Draw_withMomentum_Dis(double Theta_Tem,double Phi_Tem,int Charge_Tem,int MomentumNum_Tem,double* MomentumValue_Tem);
  TGraph* Draw_withMomentum_Dis(double Theta_Tem,double Phi_Tem,int Charge_Tem,int DrawOpt);
  
private:
  TFile* f1_DB_ClusterNum;
  
  int MomentumNum_Plus;
  int MomentumNum_Minus;
  double ThetaBin_Unit;
  double PhiBin_Unit;
  
  double Momentum_Range_Plus[2];
  double Momentum_Range_Minus[2];
  
  double MomentumValue_Plus[MaxMomentumNum_Plus];
  double ClusterNum_Plus[THETANUM][PHINUM][MaxMomentumNum_Plus];
  
  double MomentumValue_Minus[MaxMomentumNum_Minus];
  double ClusterNum_Minus[THETANUM][PHINUM][MaxMomentumNum_Minus];
  
  ClassDef(ST_ClusterNum_DB,1)
};

#endif
