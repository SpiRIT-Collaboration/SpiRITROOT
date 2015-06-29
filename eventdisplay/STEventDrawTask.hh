/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#pragma once

// FairRoot classes
#include "FairTask.h"
#include "FairLogger.h"

// ROOT classes
#include "TEveBoxSet.h"
#include "TEvePointSet.h"
#include "TClonesArray.h"
#include "TVector3.h"

#include "TCanvas.h"
#include "TH2D.h"
#include "TTree.h"
#include "TGraph.h"

#include "STEventManager.hh"
#include "STEventManagerEditor.hh"
#include "STRiemannTrack.hh"
#include "STRiemannHit.hh"
#include "STEvent.hh"
#include "STHit.hh"
#include "STRawEvent.hh"
#include "STDigiPar.hh"
#include "STGas.hh"

class STEventDrawTask : public FairTask
{
  public :
    static STEventDrawTask* Instance();
    STEventDrawTask();
    STEventDrawTask(TString modes);

    virtual ~STEventDrawTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t* option);
    virtual void SetParContainers();
    void Reset();

    void Set2DPlotRange(Int_t uaIdx);
    void SetThreshold(Int_t val) { fThreshold=val; }

    void SetHitAttributes(Color_t, Size_t, Style_t);
    void SetHitClusterAttributes(Color_t, Size_t, Style_t);
    void SetRiemannAttributes(Color_t, Size_t, Style_t);
    void SetSelfRiemannSet(Int_t iRiemannSet = -1, Bool_t offElse = kTRUE);

    //void SetDigiFile(TString name);
    static void ClickSelectedPadPlane();
    void DrawPad(Int_t row, Int_t layer);
    void DrawPadByPosition(Double_t x, Double_t z);

    Int_t GetNRiemannSet() { return fRiemannSetArray.size(); };

    void SetEventManagerEditor(STEventManagerEditor* pointer)
    { fEventManagerEditor = pointer; } 

  private :
    void DrawPadPlane();
    void SetHistPad();
    void UpdateCvsPadPlane();

    void DrawHitPoints();
    void DrawHitClusterPoints();
    void DrawRiemannHits();

    Color_t GetRiemannColor(Int_t);

    Int_t fCurrentEvent;
    Int_t fCurrentRow;
    Int_t fCurrentLayer;

    Bool_t fSet2dPlotRangeFlag;
    Bool_t fSetDigiFileFlag;

    STDigiPar* fPar;

    Int_t fNTbs;
    Double_t fXPadPlane;

    Double_t fTBTime;
    Double_t fDriftVelocity;

    TH1D* fHistPad;
    TGraph* fGraphHitTb[20];

    TClonesArray* fHitArray;
    TClonesArray* fHitClusterArray;
    TClonesArray* fRiemannTrackArray;
    TClonesArray* fKalmanArray;
    TClonesArray* fRawEventArray;

    STRawEvent* fRawEvent;

    STEventManager* fEventManager;
    STEventManagerEditor* fEventManagerEditor;

    Int_t fThreshold;

    TEvePointSet* fHitSet;
    Color_t fHitColor;
    Size_t  fHitSize;
    Style_t fHitStyle;

    TEveBoxSet* fBoxClusterSet;
    Color_t fBoxClusterColor;
    Int_t fBoxClusterTransparency;

    TEvePointSet* fHitClusterSet;
    Color_t fHitClusterColor;
    Size_t  fHitClusterSize;
    Style_t fHitClusterStyle;

    vector<TEvePointSet*> fRiemannSetArray;
    Color_t fRiemannColor;
    Size_t  fRiemannSize;
    Style_t fRiemannStyle;

    TCanvas* fCvsPadPlane;
    TH2D* fPadPlane;
    Int_t fMinZ;
    Int_t fMaxZ;
    Int_t fMinX;
    Int_t fMaxX;

    TCanvas* fCvsPad;

    static STEventDrawTask* fInstance;

  ClassDef(STEventDrawTask,1);
};
