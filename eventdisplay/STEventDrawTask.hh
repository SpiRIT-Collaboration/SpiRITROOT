/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STEVENTDRAWTASK
#define STEVENTDRAWTASK

// FairRoot class headers
#include "FairTask.h"
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

// SPiRIT class headers
#include "STEventManager.hh"
#include "STEventManagerEditor.hh"
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

// STL class headers
#include <vector>
#include <iostream>

#define NUMEVEOBJ 8

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
      kMC          = 0,
      kDigi        = 1,
      kHit         = 2,
      kCluster     = 3,
      kClusterBox  = 4,
      kRiemannHit  = 5,
      kLinear      = 6,
      kLinearHit   = 7,
      kHitBox      = 8
    };

    void Set2DPlotRange(Int_t uaIdx);
    void SetRange(Double_t min, Double_t max);
    void SetThresholdRange(STEveObject eve, Double_t min, Double_t max);
    void SetWindowRange(Int_t start, Int_t end);
    void SetWindow(Int_t start = 0, Int_t num = 512);

    void SetSelfRiemannSet(Int_t iRiemannSet = -1, Bool_t offElse = kTRUE);
    void SetSelfLinearSet(Int_t iLinearSet = -1, Bool_t offElse = kTRUE);
    void SetRendering(STEveObject eveObj, 
                           Bool_t rnr = kTRUE,
                         Double_t thresholdMin = -1, 
                         Double_t thresholdMax = -1);
    void SetAttributes(STEveObject eveObj, 
                           Style_t style = -1, 
                            Size_t size  = -1, 
                           Color_t color = -1);
    void SetObject(STEveObject eveObj, Bool_t set);

    void SetDrawHitAndDrift(Bool_t val);

    static void ClickSelectedPadPlane();
    void DrawPadByPosition(Double_t x, Double_t z);
    void DrawPad(Int_t row, Int_t layer);
    void UpdatePadRange();

    Int_t GetWindowTbStart();
    Int_t GetWindowTbEnd();

    Int_t GetNRiemannSet();
    Int_t GetNLinearSet();

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
    void DrawLinearTracks();

    Color_t GetColor(Int_t);

  private :
    STEventManager* fEventManager;
    STEventManagerEditor* fEventManagerEditor;

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

    Int_t fWindowTbStart;
    Int_t fWindowTbEnd;
    Double_t fWindowYStart;
    Double_t fWindowYEnd;

    static const Int_t fNumEveObject = 9;

    Bool_t   fSetObject[fNumEveObject];
    Bool_t   fRnrSelf[fNumEveObject];
    Color_t  fEveColor[fNumEveObject];
    Size_t   fEveSize[fNumEveObject];
    Style_t  fEveStyle[fNumEveObject];
    Double_t fThresholdMin[fNumEveObject];
    Double_t fThresholdMax[fNumEveObject];

    TEvePointSet* fPointSet[fNumEveObject];

    TEveBoxSet* fBoxHitSet;
    TEveBoxSet* fBoxClusterSet;

    vector<TEvePointSet*> fRiemannSetArray;
    vector<TEvePointSet*> fLinearHitSetArray;
    vector<TEveLine*>     fLinearTrackSetArray;

    STLinearTrackFitter* fLTFitter;

    Double_t fMaxAdcCurrentPad;

    static STEventDrawTask* fInstance;

  ClassDef(STEventDrawTask,3);
};

#endif
