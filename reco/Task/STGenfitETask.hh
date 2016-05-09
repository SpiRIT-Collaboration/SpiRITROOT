#ifndef STGENFITETASK
#define STGENFITETASK

#include "STRecoTask.hh"
#include "STRiemannTrack.hh"
#include "STGenfitTestE.hh"

class STGenfitETask : public STRecoTask
{
  public:
    STGenfitETask();
    STGenfitETask(Bool_t persistence);
    ~STGenfitETask();

    void SetIterationCut(Int_t min, Int_t max);

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetTargetPlane(TVector3 position, TVector3 normal);

  private:
    TClonesArray *fTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;
    TClonesArray *fRiemannTrackArray = nullptr;
    TClonesArray *fHitClusterArray = nullptr;

    STGenfitTestE *fGenfitTest;

    Int_t fMinIterations;
    Int_t fMaxIterations;

    genfit::GFRaveVertexFactory *fVertexFactory;

    UInt_t fID = 0;

  ClassDef(STGenfitETask, 1)
};

#endif
