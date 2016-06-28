#ifndef STGENFITETASK
#define STGENFITETASK

#include "STRecoTask.hh"
#include "STHelixTrack.hh"
#include "STGenfitTestE.hh"

class STGenfitETask : public STRecoTask
{
  public:
    STGenfitETask();
    STGenfitETask(Bool_t persistence, Bool_t removeNoVertexEvent = false);
    ~STGenfitETask();

    void SetIterationCut(Int_t min, Int_t max);

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetRemoveNoVertexEvent(Bool_t val = true);

  private:
    TClonesArray *fTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fHitClusterArray = nullptr;

    Bool_t fRemoveNoVertexEvent = false;

    STGenfitTestE *fGenfitTest;

    Int_t fMinIterations;
    Int_t fMaxIterations;

    genfit::GFRaveVertexFactory *fVertexFactory;

    UInt_t fID = 0;

  ClassDef(STGenfitETask, 1)
};

#endif
