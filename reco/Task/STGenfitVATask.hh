#ifndef STGENFITVATASK_HH
#define STGENFITVATASK_HH

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

class STGenfitVATask : public STRecoTask
{
  public:
    STGenfitVATask();
    STGenfitVATask(Bool_t persistence);
    ~STGenfitVATask();

    void SetListPersistence(bool val = true) { fIsListPersistence = val; }
    void SetPersistence(bool val = true) { fIsPersistence = val; }

    void SetClusteringType(Int_t type);
    void SetConstantField();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetBeamFile(TString fileName);
    void SetInformationForBDC(Int_t runNo, Double_t offsetX, Double_t offsetY);
    void SetZtoProject(Double_t peakZ, Double_t sigma, Double_t sigmaMultiple);

  private:
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;
    TClonesArray *fCandListArray = nullptr;
    TClonesArray *fVATrackArray = nullptr;
    TClonesArray *fBDCVertexArray = nullptr;

    bool fIsListPersistence = false;
    bool fIsSamurai = true;

    TString fGFRaveVertexMethod;
    genfit::GFRaveVertexFactory *fVertexFactory;

    STGenfitTest2 *fGenfitTest;
    STPIDTest *fPIDTest;

    Int_t fClusteringType = 2;

    TString fBeamFilename = "";
    TFile *fBeamFile;
    TTree *fBeamTree;
    Double_t fZ, fAoQ, fBeta37;
    TTree *fBDCTree;
    Double_t fBDC1x, fBDC1y, fBDC2x, fBDC2y, fBDCax, fBDCby;

    Int_t fRunNo;
    Double_t fOffsetX, fOffsetY;
    STBeamEnergy *fBeamEnergy = nullptr;
    STBDCProjection *fBDCProjection = nullptr;

    Double_t fPeakZ = -11.9084;
    Double_t fSigma = 1.69675;
    Double_t fSigmaMultiple = 3;

  ClassDef(STGenfitVATask, 1)
};

#endif
