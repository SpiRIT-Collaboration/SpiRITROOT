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
  void SetPRFCheck(Bool_t opt){fPRFCheck = opt;};
  void SetPRFOpt(Int_t opt){fPRFOpt = opt;};
  void SetPRFCutFile(TString filename);  
  void SetExBFile(TString filename){ fExB_file = filename;} ;  

private:
  STCorrection *fCorrection        = nullptr;
  TClonesArray *fHitClusterArray   = nullptr;
  TClonesArray *fHitArray       = nullptr;
  TClonesArray *fHelixArray     = nullptr;
  Bool_t fDesaturate            = true;  //defualt is to desaturate
  Int_t fSatOpt                 = 1;
  Int_t fPRFOpt                 = 1;
  Int_t fExB                    = 1;
  Bool_t fExBShift              = false; //check clusters behave by prf
  Bool_t fPRFCheck              = true; //check clusters behave by prf
  TString fPRFcut_file = "";
  TString fExB_file = "";
  
  ClassDef(STCorrectionTask, 1)
};

#endif
