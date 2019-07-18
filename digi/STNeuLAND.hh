#ifndef STNEULAND_HH
#define STNEULAND_HH

#include "TMath.h"
#include "TVector3.h"

class STNeuLAND
{
  public:
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
    Double_t fWidthBar = 50.;                ///< Size of neuland bar in z (= fdzNl/fNumLayers)
    Double_t fLengthBar = 2500.;             ///< Half length of neuland bar
    Double_t fHalfLengthBar = fLengthBar/2.; ///< Half length of neuland bar

    //Double_t fdzNlBar = 50.; ///< Size of neuland bar in z (= fdzNl/fNumLayers)
    //Double_t fdhwNlBar = 1250.; ///< Half length of neuland bar


    /// Convert local position to global position
    TVector3 GlobalPos(TVector3 localPos);

    /// Convert global position to local position
    TVector3 LocalPos(TVector3 globalPos);

    /// Get layer from bar-id
    Int_t GetLayer(Int_t mcDetID);

    /// Get row from bar-id
    Int_t GetRow(Int_t mcDetID);

    Int_t IsAlongXNotY(Int_t mcDetID);

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


  ClassDef(STNeuLAND,1);
};

#endif
