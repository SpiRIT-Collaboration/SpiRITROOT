#ifndef STGENFITPIDTASK_HH
#define STGENFITPIDTASK_HH

#include "TMatrixTSym.h"
#include "STRecoTask.hh"
#include "STHelixTrack.hh"
#include "STGenfitTest2.hh"
#include "STPIDTest.hh"
#include "STRecoTrack.hh"
#include "STRecoTrackCand.hh"
#include "STRecoTrackCandList.hh"
#include "STVertex.hh"

class STGenfitPIDTask : public STRecoTask
{
  public:
    STGenfitPIDTask();
    STGenfitPIDTask(Bool_t persistence);
    ~STGenfitPIDTask();

    void SetListPersistence(bool val = true) { fIsListPersistence = val; }
    void SetPersistence(bool val = true) { fIsPersistence = val; }

    void SetClusteringType(Int_t type);
    void SetConstantField();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetBDCFile(TString fileName);

  private:
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fCandListArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;

    bool fIsListPersistence = false;
    bool fIsSamurai = true;

    TString fGFRaveVertexMethod;
    genfit::GFRaveVertexFactory *fVertexFactory;

    STGenfitTest2 *fGenfitTest;
    STPIDTest *fPIDTest;

    Int_t fClusteringType = 2;

    TString fBDCName = "";
    TFile *fFileBDC;
    TTree *fTreeBDC;
    Double_t fXBDC, fYBDC, fZBDC;
    Double_t fdXBDC, fdYBDC, fdZBDC;
    TMatrixDSym *fCovMatBDC;

  ClassDef(STGenfitPIDTask, 1)
};

#endif
