#ifndef STGENFITSINGLEPIDTASK
#define STGENFITSINGLEPIDTASK

#include "STRecoTask.hh"
#include "STHelixTrack.hh"
#include "STGenfitTestE.hh"
#include "STDatabasePDG.hh"

class STGenfitSinglePIDTask : public STRecoTask
{
  public:
    STGenfitSinglePIDTask();
    STGenfitSinglePIDTask(Bool_t persistence);
    ~STGenfitSinglePIDTask();

    void SetIterationCut(Int_t min, Int_t max);
    void SetClusteringType(Int_t type);

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fTrackPreArray = nullptr;
    TClonesArray *fTrackArray = nullptr;
    TClonesArray *fTrackCandArray = nullptr;

    TClonesArray *fVertexArray = nullptr;

    STGenfitTestE *fGenfitTest;

    Int_t fMinIterations = 5;
    Int_t fMaxIterations = 20;

    TString fGFRaveVertexMethod;
    genfit::GFRaveVertexFactory *fVertexFactory;

    STDatabasePDG* fPDGDB;

    Int_t fClusteringType = 2;

  ClassDef(STGenfitSinglePIDTask, 1)
};

#endif
