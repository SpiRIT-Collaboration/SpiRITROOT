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
#include "TF2.h"
#include "TF1.h"

#include <fstream>
using namespace std;

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

    void SetClusterSigmaFile(TString fileName);
    void SetUseConstCov(Double_t x, Double_t y, Double_t z);
    void SetVertexMethod(TString method/*="avf-smoothing:1-Tini:256-ratio:0.25-sigmacut:5"*/);

    void SelectPID(Int_t pid);

    void SetTrackFileName(TString name);
    void SetClusterFileName(TString name);

    void FinishTask();

    void SetPrintFittedPoints(bool val) { fPrintFittedPoints = val; }
    void SetDebugVertex(TString fileName) { fDebugVertexFileName = fileName; }

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

    // Target plane position in mm.
    // Default position is set from the dimension measurement.
    Double_t fTargetX = 0;
    Double_t fTargetY = -205.5;
    Double_t fTargetZ = -13.2;

    Double_t fMaxDCluster = 9999;

    Double_t fFieldXOffset = -0.1794;
    Double_t fFieldYOffset = -20.5502;
    Double_t fFieldZOffset = 58.0526;

    TString fSigFileName = "";
    TF1 *fHitClusterSigma[2][3][3];

    Bool_t fUseConstCov = false;
    Double_t fCovX = 1.; // mm
    Double_t fCovY = 1.; // mm
    Double_t fCovZ = 1.; // mm

    Int_t fSelectPID = -1;

    TString fNameTrack;
    TFile *fFileTrack;
    TTree *fTreeTrack;
    Float_t fTrackPValue;
    Float_t fTrackWeight;
    Float_t fTrackP;
    Float_t fTrackTheta;
    Float_t fTrackPhi;
    Float_t fTrackdEdx;
    Float_t fTrackChi2;
    Float_t fTrackNDF;
    Float_t fTrackDist;
    Float_t fTrackVertexZ;
    Float_t fTrackNumClusters;

    TString fNameCluster;
    TFile *fFileCluster;
    TTree *fTreeCluster;
    TH1D *fHistRawResiduals[2][3][3];
    TH1D *fHistStdResiduals[2][3][3];
    Bool_t fClusterIsLayerOrRow;
    Float_t fClusterResidualX;
    Float_t fClusterResidualY;
    Float_t fClusterResidualZ;
    Float_t fClusterChi;
    Float_t fClusterDip;
    Int_t fClusterNumHits;
    Float_t fClusterX;
    Float_t fClusterZ;
    Int_t fCountFilledEvents = 0;

    bool fPrintFittedPoints = 0;

    TString fDebugVertexFileName ="";
    ofstream fDebugVertexFile;

  ClassDef(STGenfitPIDTask, 1)
};

#endif
