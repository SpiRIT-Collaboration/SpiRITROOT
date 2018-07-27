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
  void SetDesaturation(Bool_t opt);  
private:
  STCorrection *fCorrection     = nullptr;
  TClonesArray *fClusterArray   = nullptr;
  TClonesArray *fHitArray       = nullptr;
  Bool_t fDesaturate            = true;  //defualt is to desaturate

  ClassDef(STCorrectionTask, 1)
};

#endif
