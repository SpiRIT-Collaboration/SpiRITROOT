#ifndef STNEULAND_HH
#define STNEULAND_HH

namespace STNeuLAND
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
    Int_t fLastCDetectorID = 4400;

    Int_t fNumLayers = 8;
    Int_t fNumRows = 50;

    Double_t fdzNl = 400.;                   ///< Length of neuland array in z-direction
    Double_t fWidthBar = 50.;                ///< Size of neuland bar in z (= fdzNl/fNumLayers)
    Double_t fLengthBar = 2500.;             ///< Half length of neuland bar
    Double_t fHalfLengthBar = fLengthBar/2.; ///< Half length of neuland bar

    //Double_t fdzNlBar = 50.; ///< Size of neuland bar in z (= fdzNl/fNumLayers)
    //Double_t fdhwNlBar = 1250.; ///< Half length of neuland bar




    /// Convert local position to global position
    TVector3
    GlobalPos(TVector3 localPos)
    {
      TVector3 globalPos = localPos;
      globalPos.RotateY( fRotYNeuland_rad );
      globalPos += TVector3(fOffxNeuland, fOffyNeuland, fOffzNeuland);
      return globalPos;
    }



    /// Convert global position to local position
    TVector3
    LocalPos(TVector3 globalPos)
    {
      TVector3 localPos = globalPos;
      localPos -= TVector3(fOffxNeuland, fOffyNeuland, fOffzNeuland);
      localPos.RotateY( -fRotYNeuland_rad );
      return localPos;
    }



    /// Get row from bar-id
    Int_t
    GetRow(Int_t barID)
    {
      auto layer = GetLayer(barID);
      if (layer < 0)
        return -1;
      Int_t row = id - layer*fWidthBar;
      return row;
    }



    /// Get layer from bar-id
    Int_t
    GetLayer(Int_t barID)
    {
      if (id < fFirstMCDetectorID && id >= fLastCDetectorID ) {
        Int_t layer = Int_t((id-fFirstMCDetectorID)/fWidthBar);
        return layer;
      }
      return -1;
    }



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
    TVector3
    GetBarLocalPosition(Int_t detIDMC)
    {
      auto layer = GetLayer(id);
      auto row = GetRow(id);

      TVector3 localPosition(0,0,0);
      if (layer > 0 && row > 0) {
        localPosition.SetZ((layer+.5)*fdzNlBar);
        if (layer%2==0) localPosition.SetY(-fHalfLengthBar+(row+1)*fWidthBar);
        else            localPosition.SetX(-fHalfLengthBar+(row+1)*fWidthBar);
      }

      return localPosition;
    }



    /// Get (global) center position from mc-det-id
    TVector3
    GetBarGlobalPosition(Int_t detIDMC)
    {
      return LocalPos(GetBarLocalPosition(id));
    }
};

#endif
