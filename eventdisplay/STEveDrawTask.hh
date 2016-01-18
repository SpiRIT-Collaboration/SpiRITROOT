/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STEVEDRAWTASK
#define STEVEDRAWTASK

// FairRoot class headers
#include "FairTask.h"
#include "STEveTask.hh"
#include "FairLogger.h"

// ROOT class headers
#include "TEvePointSet.h"
#include "TEveManager.h"
#include "TEveBoxSet.h"
#include "TEveLine.h"
#include "TClonesArray.h"
#include "TPaletteAxis.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TTree.h"
#include "TH2D.h"
#include "TF1.h"

// SPiRIT class headers
#include "STEveManager.hh"
#include "STRiemannTrack.hh"
#include "STLinearTrackFitter.hh"
#include "STLinearTrack.hh"
#include "STRiemannHit.hh"
#include "STRawEvent.hh"
#include "STDigiPar.hh"
#include "STEvent.hh"
#include "STMCPoint.hh"
#include "STDriftedElectron.hh"
#include "STHit.hh"
#include "STPad.hh"
#include "STPulse.hh"

// STL class headers
#include <vector>
#include <iostream>

class STEveDrawTask : public STEveTask
{
  public :
    static STEveDrawTask* Instance();
    STEveDrawTask();
    STEveDrawTask(TString modes);

    virtual void DrawADC(Int_t row, Int_t layer);
    virtual void UpdateWindowTb(Int_t start, Int_t end);
    virtual void PushParameters();
    virtual Int_t RnrEveObject(TString name, Int_t option);
    virtual Int_t IsSet(TString name, Int_t option);

    virtual ~STEveDrawTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t* option);
    virtual void SetParContainers();
    void Reset();

    void Set2DPlotRange(Int_t uaIdx);

    void SetSelfRiemannSet(Int_t iRiemannSet = -1, Bool_t offElse = kTRUE);
    void SetSelfLinearSet(Int_t iLinearSet = -1, Bool_t offElse = kTRUE);
    void SetObject(TString name, Bool_t set);
    void SetRendering(TString name, 
                       Bool_t rnr = kTRUE,
                     Double_t min = -1, 
                     Double_t max = -1);
    void SetAttributes(TString name, 
                       Style_t style = -1, 
                        Size_t size  = -1, 
                       Color_t color = -1);
    void SetThresholdRange(TString name, Double_t min, Double_t max);

    static void ClickSelectedPadPlane();
    void DrawPadByPosition(Double_t x, Double_t z);
    void DrawPad(Int_t row, Int_t layer);
    void UpdatePadRange();

    Int_t GetWindowTbStart();
    Int_t GetWindowTbEnd();

    Int_t RenderMC(Int_t option);
    Int_t RenderDigi(Int_t option);
    Int_t RenderHit(Int_t option);
    Int_t RenderHitBox(Int_t option);
    Int_t RenderCluster(Int_t option);
    Int_t RenderClusterBox(Int_t option);
    Int_t RenderRiemannHit(Int_t option);
    Int_t RenderLinear(Int_t option);
    Int_t RenderLinearHit(Int_t option);

  private :
    void DrawPadPlane();
    void SetHistPad();
    void UpdateCvsPadPlane();

    void DrawMCPoints();
    void DrawDriftedElectrons();
    void DrawHitPoints();
    void DrawHitClusterPoints();
    void DrawRiemannHits();
    void DrawLinearTracks();

    Color_t GetColor(Int_t);

    Int_t BoolToInt(Bool_t val);

  private :
    STEveManager* fEveManager;

    STEvent* fEvent;

    TClonesArray* fMCHitArray;
    TClonesArray* fDriftedElectronArray;
    TClonesArray* fEventArray;
    TClonesArray* fRiemannTrackArray;
    TClonesArray* fLinearTrackArray;
    TClonesArray* fRawEventArray;

    STRawEvent* fRawEvent;

    Bool_t fSet2dPlotRangeFlag;
    Bool_t fSetDigiFileFlag;

    Long64_t fCurrentEvent;
    Int_t fCurrentRow;
    Int_t fCurrentLayer;

    STDigiPar* fPar;

    Int_t    fNTbs;
    Double_t fXPadPlane;
    Double_t fTBTime;
    Double_t fDriftVelocity;

    TCanvas* fCvsPad;
    TH1D* fHistPad;
    Double_t Pulse(Double_t *x, Double_t *par);
    static const Int_t fNumPulseFunction = 10;
    TF1* fPulseFunction[fNumPulseFunction];
    TGraph* fPulseSum;
    STPulse* fPulse;
    Double_t fRangeMin;
    Double_t fRangeMax;

    TCanvas* fCvsPadPlane;
    TH2D* fPadPlane;
    Int_t fMinZ;
    Int_t fMaxZ;
    Int_t fMinX;
    Int_t fMaxX;

    Int_t fWindowTbStart;
    Int_t fWindowTbEnd;
    Double_t fWindowYStart;
    Double_t fWindowYEnd;

    static const Int_t fNumEveObject = 10;

    Bool_t   fSetObject[fNumEveObject];
    Bool_t   fRnrSelf[fNumEveObject];
    Color_t  fEveColor[fNumEveObject];
    Size_t   fEveSize[fNumEveObject];
    Style_t  fEveStyle[fNumEveObject];
    Double_t fThresholdMin[fNumEveObject];
    Double_t fThresholdMax[fNumEveObject];

    TEvePointSet* fPointSet[fNumEveObject];

    TEveRGBAPalette *fRGBAPalette;

    TEveBoxSet* fBoxHitSet;
    TEveBoxSet* fBoxClusterSet;

    vector<TEvePointSet*> fRiemannSetArray;
    vector<TEvePointSet*> fLinearHitSetArray;
    vector<TEveLine*>     fLinearTrackSetArray;

    STLinearTrackFitter* fLTFitter;

    Double_t fMaxAdcCurrentPad;

    static STEveDrawTask* fInstance;

  ClassDef(STEveDrawTask, 1);
};

#endif
