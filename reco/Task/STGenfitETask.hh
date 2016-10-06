#ifndef STGENFITETASK
#define STGENFITETASK

#include "STRecoTask.hh"
#include "STHelixTrack.hh"
#include "STGenfitTestE.hh"
#include "STDatabasePDG.hh"

class STGenfitETask : public STRecoTask
{
  public:
    STGenfitETask();
    STGenfitETask(Bool_t persistence);
    ~STGenfitETask();

    void SetIterationCut(Int_t min, Int_t max);
    void SetClusteringType(Int_t type);

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    TClonesArray *fTrackArray = nullptr;
    TClonesArray *fTrackCandArray = nullptr;
    TClonesArray *fVertexArray = nullptr;
    TClonesArray *fHelixTrackArray = nullptr;

    STGenfitTestE *fGenfitTest;

    Int_t fMinIterations = 5;
    Int_t fMaxIterations = 20;

    TString fGFRaveVertexMethod;
    genfit::GFRaveVertexFactory *fVertexFactory;

    STDatabasePDG* fPDGDB;

    Int_t fClusteringType = 0;

  ClassDef(STGenfitETask, 1)
};

#endif
