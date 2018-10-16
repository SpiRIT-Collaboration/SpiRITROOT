#ifndef STEMBEDCORRELATOR_HH
#define STEMBEDCORRELATOR_HH

#include "STMCTrack.h"
#include "STRecoTask.hh"
#include "STRecoTrack.hh"
#include "TF1.h"
#include "TMath.h"

class STEmbedCorrelatorTask : public STRecoTask
{
public:
  STEmbedCorrelatorTask(double t_threshold=0);
  STEmbedCorrelatorTask(Bool_t persistence);
  ~STEmbedCorrelatorTask();
  virtual InitStatus Init();
  virtual void Exec(Option_t *opt);
  Bool_t CheckMomCorr(STMCTrack *, STRecoTrack *);
  void SetPersistence(Bool_t value){ fIsPersistence = value; }
  
private:
  bool fIsPersistence = false;
  
  TClonesArray *fRecoTrackArray = nullptr;
  TClonesArray *fMCTrackArray = nullptr;
  TClonesArray *fEmbedTrackArray = nullptr;
  
  double threshold_;  // minimum fraction of hits that needs to be an embedded 
  
  ClassDef(STEmbedCorrelatorTask, 1)
};

#endif
