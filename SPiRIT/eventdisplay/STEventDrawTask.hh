/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#pragma once

// FairRoot classes
#include "FairTask.h"
#include "FairLogger.h"

// ROOT classes
#include "TEvePointSet.h"
#include "TClonesArray.h"
#include "TVector3.h"

#include "TCanvas.h"
#include "TH2D.h"
#include "TGraph.h"

#include "STEventManager.hh"
#include "STRiemannTrack.hh"
#include "STRiemannHit.hh"
#include "STEvent.hh"
#include "STHit.hh"

class STEventDrawTask : public FairTask
{
  public :
    STEventDrawTask();
    STEventDrawTask(TString modes);

    virtual ~STEventDrawTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t* option);
    void Reset();

    void Set2DPlotRange(Int_t uaIdx);
    void SetThreshold(Int_t val) { fThreshold=val; }

    void SetHitAttributes(Color_t, Size_t, Style_t);
    void SetHitClusterAttributes(Color_t, Size_t, Style_t);
    void SetRiemannAttributes(Color_t, Size_t, Style_t);

  private :
    void DrawPadPlane();
    void UpdateCvsPadPlane();

    void DrawHitPoints();
    void DrawHitClusterPoints();
    void DrawRiemannHits();

    Bool_t fIs2DPlotRange;

    TClonesArray* fHitArray;
    TClonesArray* fHitClusterArray;
    TClonesArray* fRiemannTrackArray;
    TClonesArray* fKalmanArray;

    STEventManager* fEventManager;

    Int_t fThreshold;

    TEvePointSet* fHitSet;
    Color_t fHitColor;
    Size_t  fHitSize;
    Style_t fHitStyle;

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

    ClassDef(STEventDrawTask,1);
};
