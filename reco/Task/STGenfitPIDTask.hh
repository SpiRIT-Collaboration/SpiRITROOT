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
    void SetFieldOffset(Double_t xOffset, Double_t yOffset, Double_t zOffset);

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetBDCFile(TString fileName);

    void SetTargetPlane(Double_t x, Double_t y, Double_t z);

    void SetMaxDCluster(Double_t val) { fMaxDCluster = val; }

    genfit::GFRaveVertexFactory *GetVertexFactoryInstance();

  private:
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fCandListArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fEmbedTrackArray = nullptr;
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

    // Target plane position in mm.
    // Default position is set from the dimension measurement.
    Double_t fTargetX = 0;
    Double_t fTargetY = -205.5;
    Double_t fTargetZ = -13.2;

    Double_t fMaxDCluster = 9999;

    Double_t fFieldXOffset = -0.1794;
    Double_t fFieldYOffset = -20.5502;
    Double_t fFieldZOffset = 58.0526;

  ClassDef(STGenfitPIDTask, 1)
};

#endif
