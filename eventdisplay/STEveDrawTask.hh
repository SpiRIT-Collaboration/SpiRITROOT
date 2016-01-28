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
    /// Eve Manager pointer
    STEveManager* fEveManager = NULL;

    /// Number entries
    Long64_t fCurrentEvent = -2;
    Int_t    fCurrentRow   = -1;
    Int_t    fCurrentLayer = -1;

    /// Flags
    Bool_t fSet2dPlotRangeFlag = kFALSE;

    /// DigiPar parameters
    STDigiPar* fPar = NULL;

    Int_t    fNTbs;
    Double_t fXPadPlane;
    Double_t fTBTime;
    Double_t fDriftVelocity;

    Int_t    fWindowTbStart;
    Int_t    fWindowTbEnd;
    Double_t fWindowYStart;
    Double_t fWindowYEnd;

    /// Containers
    STEvent* fEvent = NULL;
    STRawEvent* fRawEvent = NULL;

    TClonesArray* fMCHitArray           = NULL;
    TClonesArray* fDriftedElectronArray = NULL;
    TClonesArray* fEventArray           = NULL;
    TClonesArray* fRiemannTrackArray    = NULL;
    TClonesArray* fLinearTrackArray     = NULL;
    TClonesArray* fRawEventArray        = NULL;

    /// Pad & Pulses
    static const Int_t fNumPulseFunction = 10;
    TF1 *fPulseFunction[fNumPulseFunction];
    Double_t Pulse(Double_t *x, Double_t *par);

    TCanvas *fCvsPad   = NULL;
    TH1D    *fHistPad  = NULL;
    TGraph  *fPulseSum = NULL;
    STPulse *fPulse    = NULL;

    Double_t fRangeMin = 0;
    Double_t fRangeMax = 0;

    /// Pad-Plane
    TCanvas* fCvsPadPlane = NULL;
    TH2D* fPadPlane       = NULL;

    Int_t fMinZ =    0; 
    Int_t fMaxZ = 1344;
    Int_t fMinX =  432;
    Int_t fMaxX = -432;

    /// Objects
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

    TEveBoxSet* fBoxHitSet     = NULL;
    TEveBoxSet* fBoxClusterSet = NULL;

    vector<TEvePointSet*> fRiemannSetArray;
    vector<TEvePointSet*> fLinearHitSetArray;
    vector<TEveLine*>     fLinearTrackSetArray;

    STLinearTrackFitter* fLTFitter = NULL;


    static STEveDrawTask* fInstance;

  ClassDef(STEveDrawTask, 1);
};

#endif
