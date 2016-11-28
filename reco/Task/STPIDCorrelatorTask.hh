#ifndef STPIDCORRELATORTASK_HH
#define STPIDCORRELATORTASK_HH

#include "STRecoTask.hh"
#include "TF1.h"
#include "TMath.h"

class STPIDCorrelatorTask : public STRecoTask
{
public:
  STPIDCorrelatorTask();
  STPIDCorrelatorTask(Bool_t persistence);
  ~STPIDCorrelatorTask();
  
  static  Double_t fitdEdxPion(Double_t *x, Double_t *par);
  static  Double_t fitdEdxProton(Double_t *x, Double_t *par);
  static  Double_t fitdEdxDeuterium(Double_t *x, Double_t *par);
  static  Double_t fitdEdxTriton(Double_t *x, Double_t *par);
  static  Double_t fitdEdx3He(Double_t *x, Double_t *par);
  static  Double_t fitdEdx4He(Double_t *x, Double_t *par);
  static  Double_t GetMu(TString particle, Double_t p);
  static  Double_t GetSigma(TString particle, Double_t p);
  static  Double_t GetPdf(Double_t dedx, Double_t mu, Double_t sigma);
  
  virtual InitStatus Init();
  virtual void Exec(Option_t *opt);
  
private:
  TClonesArray *fRecoTrackArray = nullptr;
  TClonesArray *fPIDProbArray = nullptr;
  
  ClassDef(STPIDCorrelatorTask, 1)
};

#endif
