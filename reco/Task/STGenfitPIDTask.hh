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
#include "STBeamEnergy.hh"
#include "STBDCProjection.hh"

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

    void SetTargetPlane(Double_t x, Double_t y, Double_t z);

    void SetMaxDCluster(Double_t val) { fMaxDCluster = val; }

    genfit::GFRaveVertexFactory *GetVertexFactoryInstance();

  private:
    TClonesArray *fHitClusterArray = nullptr;
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fCandListArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;
    TClonesArray *fBDCTrackArray = nullptr;

    bool fIsListPersistence = false;
    bool fIsSamurai = true;

    TString fGFRaveVertexMethod;
    genfit::GFRaveVertexFactory *fVertexFactory;

    STGenfitTest2 *fGenfitTest;
    STPIDTest *fPIDTest;

    Int_t fClusteringType = 2;

    TString fBDCName = "";

    TFile *fBeamFile;
    TTree *fBeamTree;
    Int_t fABeam;
    Double_t fZ, fAoQ, fBeta37;
    TFile *fBDCFile;
    TTree *fBDCTree;
    Double_t fBDC1x, fBDC1y, fBDC2x, fBDC2y, fBDCax, fBDCby;
    Double_t fPBDC;

    Int_t fRunNo;
    Double_t fOffsetX, fOffsetY;
    STBeamEnergy *fBeamEnergy = nullptr;
    STBDCProjection *fBDCProjection = nullptr;

    TMatrixDSym *fCovMatBDC;

    // Target plane position in mm.
    // Default position is set from the dimension measurement.
    Double_t fTargetX = 0;
    Double_t fTargetY = -213.3;
    Double_t fTargetZ = -8.9;

    Double_t fMaxDCluster = 9999;

  ClassDef(STGenfitPIDTask, 1)
};

#endif
