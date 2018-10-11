
#ifndef STCORRECTION_HH
#define STCORRECTION_HH

#include "STRecoTask.hh"
#include "STRecoTrack.hh"
#include "STHitCluster.hh"
#include "STHit.hh"
#include "STCorrection.hh"

class STCorrectionTask : public STRecoTask
{
public:
  STCorrectionTask();
  STCorrectionTask(Bool_t persistence);
  ~STCorrectionTask();
  virtual InitStatus Init();
  virtual void Exec(Option_t *opt);

  void SetDesaturation(Bool_t opt){fDesaturate = opt;};  
  void SetDesatOpt(Int_t opt){fSatOpt = opt;};  
  void SetPRFCutFile(TString filename);  

private:
  STCorrection *fCorrection        = nullptr;
  TClonesArray *fHitClusterArray   = nullptr;
  TClonesArray *fHitArray       = nullptr;
  TClonesArray *fHelixArray     = nullptr;
  Bool_t fDesaturate            = true;  //defualt is to desaturate
  Int_t fSatOpt                 = 0;
  Bool_t fPRFCheck              = false; //check clusters behave by prf
  TString fPRFcut_file = "";

  ClassDef(STCorrectionTask, 1)
};

#endif
