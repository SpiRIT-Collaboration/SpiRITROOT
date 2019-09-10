#ifndef STMCTRUTHTASK_HH
#define STMCTRUTHTASK_HH

#include "STVertex.hh"
#include "STRecoTask.hh"

class STMCTruthTask : public STRecoTask
{
  public:
    STMCTruthTask();
    STMCTruthTask(Bool_t persistence);
    ~STMCTruthTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    TClonesArray *fMCTruthArray = nullptr;

    TClonesArray *fMCTrackArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fMCVertex = nullptr;

    TClonesArray *fMCArray;
    TClonesArray *fRecoArray;

  ClassDef(STMCTruthTask, 1)
};

#endif
