/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#pragma once

// FairRoot class headers
#include "FairTask.h"
#include "FairLogger.h"
#include "FairRunAna.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

// ROOT class headers
#include "TEvePointSet.h"
#include "TEveManager.h"
#include "TEveBoxSet.h"
#include "TClonesArray.h"
#include "TPaletteAxis.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TTree.h"
#include "TH2D.h"

// SPiRIT class headers
#include "STEventManager.hh"
#include "STEventManagerEditor.hh"
#include "STRiemannTrack.hh"
#include "STRiemannHit.hh"
#include "STRawEvent.hh"
#include "STDigiPar.hh"
#include "STEvent.hh"
#include "STMCPoint.hh"
#include "STDriftedElectron.hh"
#include "STHit.hh"
#include "STPad.hh"
#include "STGas.hh"

// STL class headers
#include <vector>
#include <iostream>

using namespace std;


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

    enum STEveObject
    {
      kMC      = 0,
      kDigi    = 1,
      kHit     = 2,
      kCluster = 3,
      kRiemann = 4,
      kClusterBox = 9
    };

    void Set2DPlotRange(Int_t uaIdx);
    void SetRange(Double_t min, Double_t max);

    void SetSelfRiemannSet(Int_t iRiemannSet = -1, Bool_t offElse = kTRUE);
    void SetRendering(STEveObject eveObj, Bool_t rnr, Double_t thresholdMin = -1, Double_t thresholdMax = -1);
    void SetAttributes(STEveObject eveObj, Style_t style = -1, Size_t size = -1, Color_t color = -1);

    static void ClickSelectedPadPlane();
    void DrawPad(Int_t row, Int_t layer);
    void DrawPadByPosition(Double_t x, Double_t z);

    Int_t GetNRiemannSet();

    void SetEventManagerEditor(STEventManagerEditor* pointer);

  private :
    void DrawPadPlane();
    void SetHistPad();
    void UpdateCvsPadPlane();

    void DrawMCPoints();
    void DrawDriftedElectrons();
    void DrawHitPoints();
    void DrawHitClusterPoints();
    void DrawRiemannHits();

    Color_t GetRiemannColor(Int_t);

    STEventManager* fEventManager;
    STEventManagerEditor* fEventManagerEditor;

    TClonesArray* fMCHitArray;
    TClonesArray* fDriftedElectronArray;
    TClonesArray* fHitArray;
    TClonesArray* fHitClusterArray;
    TClonesArray* fRiemannTrackArray;
    TClonesArray* fKalmanArray;
    TClonesArray* fRawEventArray;

    STRawEvent* fRawEvent;

    Bool_t fSet2dPlotRangeFlag;
    Bool_t fSetDigiFileFlag;

    Int_t fCurrentEvent;
    Int_t fCurrentRow;
    Int_t fCurrentLayer;

    STDigiPar* fPar;

    Int_t    fNTbs;
    Double_t fXPadPlane;
    Double_t fTBTime;
    Double_t fDriftVelocity;

    TCanvas* fCvsPad;
    TH1D* fHistPad;
    Double_t fRangeMin;
    Double_t fRangeMax;

    TCanvas* fCvsPadPlane;
    TH2D* fPadPlane;
    Int_t fMinZ;
    Int_t fMaxZ;
    Int_t fMinX;
    Int_t fMaxX;

    TEvePointSet* fPointSet[10];
    Bool_t   fRnrSelf[10];
    Color_t  fPointColor[10];
    Size_t   fPointSize[10];
    Style_t  fPointStyle[10];
    Double_t fThresholdMin[10];
    Double_t fThresholdMax[10];

    TEveBoxSet* fBoxClusterSet;
    vector<TEvePointSet*> fRiemannSetArray;

    static STEventDrawTask* fInstance;

  ClassDef(STEventDrawTask,2);
};
