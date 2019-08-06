#ifndef STNEULAND_HH
#define STNEULAND_HH

#include "TMath.h"
#include "TVector3.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TText.h"
#include "TLatex.h"

#include <vector>

using namespace std;

class STNeuLAND
{
  public:
    static STNeuLAND* GetNeuLAND();

    STNeuLAND();

    //////////////////////////////////////////////////////////

    Int_t fNumTDCBins = 100;

    Double_t fDecayTime = 2.1; ///< decay time of the pulse  [ns]
    Double_t fRiseTime = 0.01; ///< rise time of the pulse  [ns]
    Double_t fTimeErrorSigma = 0.15; ///< time resolution error sigma (of gaussian) [ns]
    Double_t fEffc = 140.; ///< [mm/ns]

    Int_t fPulseTDCRange = 0;

    Double_t ConvertTDCToTime(Double_t tdc) const { return tdc / fNumTDCBins * fLengthBar / fEffc; }
    Double_t ConvertTimeToTDC(Double_t time) const { return time * fEffc / fLengthBar * fNumTDCBins; }

    Double_t PulseWithError(double *xx, double *pp);
    TF1  *fFuncPulse = nullptr; //!

    TH1D *fHistPulse = nullptr; //! < for fast calculation
    TH1D *GetHistPulse() { return fHistPulse; }

    //////////////////////////////////////////////////////////

    Double_t fZTarget = -13.24; // mm
    Double_t fDistNeuland = 9093.85;
    Double_t fRotYNeuland_deg = 29.579;
    Double_t fRotYNeuland_rad = fRotYNeuland_deg*TMath::DegToRad();
    Double_t fOffxNeuland = fDistNeuland * sin( fRotYNeuland_rad ); 
    Double_t fOffyNeuland = 0.;   
    Double_t fOffzNeuland = fDistNeuland * cos( fRotYNeuland_rad ) + fZTarget;

    Int_t fFirstMCDetectorID = 4000;
    Int_t fLastMCDetectorID = 4400;

    Int_t fNumLayers = 8;
    Int_t fNumRows = 50;

    Double_t fdzNl = 400.;                   ///< Length of neuland array in z-direction
    Double_t fdzLayer = fdzNl/fNumLayers;  ///< Length of neuland array in z-direction
    Double_t fWidthBar = 50.;                ///< Size of neuland bar in z (= fdzNl/fNumLayers)
    Double_t fLengthBar = 2500.;             ///< Half length of neuland bar
    Double_t fHalfLengthBar = fLengthBar/2.; ///< Half length of neuland bar


    /// Convert local position to global position
    TVector3 GlobalPos(TVector3 localPos);

    /// Convert global position to local position
    TVector3 LocalPos(TVector3 globalPos);

    /// Get layer from bar-id
    Int_t GetLayer(Int_t mcDetID);

    /// Get row from bar-id
    Int_t GetRow(Int_t mcDetID);

    Int_t IsAlongXNotY(Int_t mcDetID);

    /// find bar ID from local position
    Int_t FindBarID(TVector3 pos);
    Int_t FindBarID(Int_t layer, Int_t row);

    /* Get (local)  center position from mc-det-id
     *
     * [neuland-bar-id]  = [mc-detector-id] - 4000
     * [neuland-veto-id] = [mc-detector-id] - 5000
     *
     * even 50s
     *   0 + 0-49 : layer 0, from bottom(0) to top(49)
     * 100 + 0-49 : layer 2, from bottom(0) to top(49)
     * 200 + 0-49 : layer 4, from bottom(0) to top(49)
     * 300 + 0-49 : layer 6, from bottom(0) to top(49)
     *
     * odd 50s
     *  50 + 0-49 : layer 1, from     -x(0) to  +x(49)
     * 150 + 0-49 : layer 3, from     -x(0) to  +x(49)
     * 250 + 0-49 : layer 5, from     -x(0) to  +x(49)
     * 250 + 0-49 : layer 7, from     -x(0) to  +x(49)
     */
    TVector3 GetBarLocalPosition(Int_t mcDetID);

    /// Get (global) center position from mc-det-id
    TVector3 GetBarGlobalPosition(Int_t mcDetID);



    //////////////////////////////////////////////////////////

    TH2D *fLocalFrameDownStream = nullptr;
    TH2D *fLocalFrameSide = nullptr;
    TGraph *fLocalNLGraphDownStream = nullptr;
    TGraph *fLocalNLGraphSideOut = nullptr;

    TLine *fGlobalNLGuideLineC = nullptr;
    TLine *fGlobalNLGuideLineR = nullptr;
    TLine *fGlobalNLGuideLineL = nullptr;
    TLatex *fGlobalNLGuideTextC = nullptr;
    TLatex *fGlobalNLGuideTextR = nullptr;
    TLatex *fGlobalNLGuideTextL = nullptr;

    TLine *fGlobalNLGuideLineT = nullptr;
    TLine *fGlobalNLGuideLineB = nullptr;
    TLatex *fGlobalNLGuideTextT = nullptr;
    TLatex *fGlobalNLGuideTextB = nullptr;

    vector<TLine *> fLocalNLGraphDownStreamArray;
    vector<TLine *> fLocalNLGraphSideArray;

    TCanvas *DrawLocal(TString name, Int_t detail=0);
    void DrawLocalFrameDownStream(Option_t *opt="");
    void DrawLocalFrameSide(Option_t *opt="");

    /// @param detail 0:full, 1:x0.1, 2:only-layer
    void DrawLocalNLGraphDownStream(Option_t *opt="", Int_t detail=0);
    /// @param detail 0:full, 1:x0.1, 2:only-layer
    void DrawLocalNLGraphSide(Option_t *opt="", Int_t detail=0);


    TH2D *fGlobalFrameTop = nullptr;
    TH2D *fGlobalFrameSide = nullptr;
    TGraph *fGlobalNLGraphTop = nullptr;
    TGraph *fGlobalNLGraphSideOut = nullptr;
    TGraph *fGLobalNLGraphSideIn = nullptr;

    TCanvas *DrawGlobal(TString name);
    void DrawGlobalFrameTop(Option_t *opt="");
    void DrawGlobalFrameSide(Option_t *opt="");
    void DrawGlobalNLGraphTop(Option_t *opt="");
    void DrawGlobalNLGraphSide(Option_t *opt="");


    //////////////////////////////////////////////////////////

  private:
    static STNeuLAND *fInstance;


  ClassDef(STNeuLAND,1);
};

#endif
