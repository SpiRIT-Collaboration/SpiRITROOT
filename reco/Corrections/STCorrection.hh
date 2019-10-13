#ifndef STCORRECTION
#define STCORRECTION

#include "STHelixTrack.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "TClonesArray.h"
#include "TMinuit.h"
#include "TFile.h"
#include "TCutG.h"
#include "TF1.h"
#include "TH3.h"
#include <vector>

typedef std::vector<STHit*>* vhit_t;

//class MyFitFunction;
class MyFitFunction
{
  static MyFitFunction* _instance_helix;
  static MyFitFunction* _instance;
  MyFitFunction() {}
  MyFitFunction(const MyFitFunction& o) {}
  MyFitFunction& operator=(const MyFitFunction& o) { return *this; }

public:
  static MyFitFunction* Instance();
  static MyFitFunction* Instance_Helix();

  Double_t Layer_norm(Double_t alpha);
  Double_t Layer_sigma(Double_t alpha);
  Double_t Row_norm(Double_t alpha);
  Double_t Row_sigma(Double_t alpha);
  
  Double_t Function(Int_t&, Double_t*, Double_t&,Double_t *, Int_t);
  Double_t Function_helix(Int_t&, Double_t*, Double_t&,Double_t *, Int_t);
  Double_t PRF(double x, double alpha);
  Double_t PRF_helix(double x, double alpha);
  
  std::vector<double> getmean(double par[]);
  void SetAryPointers(std::vector<double> *a, std::vector<double> *a_chg, std::vector<double> *b, std::vector<double> *b_chg, std::vector<STHit*> *hit_ptrs_t, std::vector<STHit*> *s_hit_ptrs_t);
  void SetLambdaChg(std::vector<double> *a, std::vector<double> *a_chg, std::vector<double> *b, std::vector<double> *b_chg, std::vector<STHit*> *hit_ptrs_t, std::vector<STHit*> *s_hit_ptrs_t);
  Bool_t byRow = false;
  Double_t cluster_alpha = 0;
  
private:
  TF1 *gaus_prf          = new TF1("gaus_prf","[0]*TMath::Gaus(x,0.,[1],false)",-30,30);
  TF1 *gaus_prf_helix    = new TF1("gaus_prf_helix","[0]*TMath::Gaus(x,0.,[1],false)",-30,30);
  std::vector<double> *hits_pos_ary   = new ::std::vector<double>;   //non saturated hit position array in cluster
  std::vector<double> *hits_chg_ary   = new ::std::vector<double>;   //non saturated hit charge array
  std::vector<double> *s_hits_pos_ary = new ::std::vector<double>; //saturated hit position array
  std::vector<double> *s_hits_chg_ary = new ::std::vector<double>; //saturated hit charge array

  std::vector<double> *hits_lambda_ary   = new ::std::vector<double>;   //non saturated hit lambda array in cluster
  std::vector<double> *s_hits_lambda_ary = new ::std::vector<double>; //saturated hit lambda array

  std::vector<STHit *> *s_hit_ptrs = new ::std::vector<STHit *>; //saturated hit ptrs
  std::vector<STHit *> *hit_ptrs   = new ::std::vector<STHit *>; //non saturated hit ptrs 
};

class STCorrection
{
  public:
  STCorrection();
  ~STCorrection() {}
  //  void SetSaturationOption(Int_t opt);
  void Desaturate(TClonesArray *clusters);
  void Desaturate_byHelix(TClonesArray *helixArray, TClonesArray *clusterArray);
  void CheckClusterPRF(TClonesArray *clusters,TClonesArray *helix, TClonesArray *hit);
  void CheckClusterPRFOld(TClonesArray *clusters,TClonesArray *helix, TClonesArray *hit);
  void CorrectExB(TClonesArray *clusters);
  void LoadPRFCut(TString filename);
  void LoadExBShift(TString filename);
  
  private:

  TH3D *exb_x   = nullptr; 
  TH3D *exb_z   = nullptr;
  TCutG *prf_row   = nullptr;
  TCutG *prf_layer = nullptr;
  TFile *exb_f = nullptr;
  
  std::vector<double> minimize(const int npar);
  std::vector<double> minimize_helix(const int npar);    

  //  TClonesArray *fTrackArray = nullptr;       ///< STHelixTrack array
  //  TClonesArray *fClusterArray = nullptr;  ///< STHitCluster array
  //  STHelixTrackFitter *fFitter = nullptr;     ///< Helix track fitter

  ClassDef(STCorrection, 3)
};

#endif
