#ifndef STCORRECTION
#define STCORRECTION

#include "STHelixTrack.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "TClonesArray.h"
#include "TMinuit.h"
#include "TFile.h"
#include "TCutG.h"
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

  Double_t Function(Int_t&, Double_t*, Double_t&,Double_t *, Int_t);
  Double_t Function_helix(Int_t&, Double_t*, Double_t&,Double_t *, Int_t);
  double PRF(double x, double par[]);
  double PRF_helix(double x, double par[]);
  
  std::vector<double> getmean(double par[]);
  void SetAryPointers(std::vector<double> *a, std::vector<double> *a_chg, std::vector<double> *b, std::vector<double> *b_chg, std::vector<STHit*> *hit_ptrs_t, std::vector<STHit*> *s_hit_ptrs_t);
  void SetLambdaChg(std::vector<double> *a, std::vector<double> *a_chg, std::vector<double> *b, std::vector<double> *b_chg, std::vector<STHit*> *hit_ptrs_t, std::vector<STHit*> *s_hit_ptrs_t);
  bool byRow = false;

private:
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
  void LoadPRFCut(TString filename);
  
  private:

  TCutG *prf_row   = nullptr;
  TCutG *prf_layer = nullptr;
  std::vector<double> minimize(const int npar);
  std::vector<double> minimize_helix(const int npar);    

  //  TClonesArray *fTrackArray = nullptr;       ///< STHelixTrack array
  //  TClonesArray *fClusterArray = nullptr;  ///< STHitCluster array
  //  STHelixTrackFitter *fFitter = nullptr;     ///< Helix track fitter

  ClassDef(STCorrection, 3)
};

#endif
