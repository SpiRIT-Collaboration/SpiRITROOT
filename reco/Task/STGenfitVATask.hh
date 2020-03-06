#ifndef STGENFITVATASK_HH
#define STGENFITVATASK_HH

#include "TMatrixTSym.h"
#include "FairRunAna.h"
#include "FairMCEventHeader.h"

#include "STFairMCEventHeader.hh"
#include "STRecoTask.hh"
#include "STGenfitPIDTask.hh"
#include "STHelixTrack.hh"
#include "STGenfitTest2.hh"
#include "STPIDTest.hh"
#include "STRecoTrack.hh"
#include "STRecoTrackCand.hh"
#include "STRecoTrackCandList.hh"
#include "STVertex.hh"
#include "STBeamEnergy.hh"
#include "STBDCProjection.hh"
#include "ST_VertexShift.hh"
#include "STBeamInfo.hh"
#include "TGraph.h"

#include <fstream>
using namespace std;

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
    void SetFieldOffset(Double_t xOffset, Double_t yOffset, Double_t zOffset);

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetBeamFile(TString fileName);
    void SetPerferMCBeam();
    void SetInformationForBDC(Int_t runNo, Double_t offsetX, Double_t offsetY, Double_t offsetZ = 0);
    void SetZtoProject(Double_t peakZ, Double_t sigma, Double_t sigmaMultiple);
    void SetFixedVertex(Double_t x, Double_t y, Double_t z);

    void SetClusterSigmaFile(TString fileName) { fSigFileName = fileName; }

    void SetUseRave(Bool_t val = kTRUE);
    void SelectPID(Int_t pid);

    void SetTrackFileName(TString name);
    void SetClusterFileName(TString name);

    void FinishTask();

    void SetPrintFittedPoints(bool val) { fPrintFittedPoints = val; }
    void SetDebugVertex(TString fileName) { fDebugVertexFileName = fileName; }

  private:
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;
    TClonesArray *fCandListArray = nullptr;
    TClonesArray *fVATrackArray = nullptr;
    TClonesArray *fBDCVertexArray = nullptr;
    TClonesArray *fVAVertexArray = nullptr;
    STBeamInfo   *fBeamInfo = nullptr;
    FairMCEventHeader *fMCEventHeader = nullptr;
    STFairMCEventHeader *fSTMCEventHeader = nullptr;

    bool fIsListPersistence = false;
    bool fIsSamurai = true;

    Bool_t fUseRave = kFALSE;
    Bool_t fUseMCBeam = kFALSE;
    genfit::GFRaveVertexFactory *fVertexFactory = nullptr;

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
    Double_t fOffsetX, fOffsetY, fOffsetZ;
    STBeamEnergy *fBeamEnergy = nullptr;
    STBDCProjection *fBDCProjection = nullptr;

    Double_t fPeakZ = -9999;
    Double_t fSigma = 0;
    Double_t fSigmaMultiple = 0;

    Double_t fFixedVertexX = -9999;
    Double_t fFixedVertexY = -9999;
    Double_t fFixedVertexZ = -9999;

    Double_t fFieldXOffset = -0.1794;  //unit: cm
    Double_t fFieldYOffset = -20.5502; //unit: cm
    Double_t fFieldZOffset = 58.0526;  //unit: cm

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

    TString fSigFileName = "";
    TF1 *fHitClusterSigma[2][3][3];

    TString fNameCluster;
    TFile *fFileCluster;
    TTree *fTreeCluster;
    TH1D *fHistRawResiduals[2][3][3][18];
    TH1D *fHistStdResiduals[3];
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

public:     
    //the below is related to BDC shift.
    ST_VertexShift* Vertex_Shifter; 
    string FileName_BDCCorrection_Theta_TargetPos;
    void Set_FileName_BDCCorrection_Theta_TargetPos(string NameTem){ FileName_BDCCorrection_Theta_TargetPos = NameTem; }
    bool IsOption_BDCCorrection;
    void Set_IsOption_BDCCorrection(bool IsOption) { IsOption_BDCCorrection = IsOption; }
    
    


};

#endif
