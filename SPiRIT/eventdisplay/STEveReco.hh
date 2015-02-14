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

#include "STEveEventManager.hh"
#include "STEvent.hh"
#include "STHit.hh"

class STEveReco : public FairTask
{
  public :
    STEveReco();
    STEveReco(TString modes);

    virtual ~STEveReco();

    virtual InitStatus Init();
    virtual void Exec(Option_t* option);
    void Reset();

    void Set2DPlotRange(Int_t uaIdx);
    void SetThreshold(Int_t val) { fThreshold=val; }

  private :
    void DrawPadPlane();
    void UpdateCvsPadPlane();

    Bool_t fIs2DPlotRange;

    TClonesArray* fHitArray;
    TClonesArray* fHitClusterArray;
    TClonesArray* fRiemannArray;
    TClonesArray* fKalmanArray;

    STEveEventManager* fEventManager;

    Int_t fThreshold;

    TEvePointSet* fHitSet;
    TEvePointSet* fHitClusterSet;

    TCanvas* fCvsPadPlane;
    TH2D* fPadPlane;
    Int_t fMinZ;
    Int_t fMaxZ;
    Int_t fMinX;
    Int_t fMaxX;

    ClassDef(STEveReco,1);
};
