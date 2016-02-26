/**
 * @brief  Track finder virtual class
 * @author JungWoo Lee
 */

#ifndef STVTRACKFINDER
#define STVTRACKFINDER

#include "STHit.hh"
#include "STEvent.hh"
#include "STLinearTrack.hh"
#include "STLinearTrackFitter.hh"

#include "TClonesArray.h"
#include "TVector3.h"

#include "FairLogger.h"

class STVTrackFinder
{
  public:
    STVTrackFinder();
    virtual ~STVTrackFinder() {} 

    /// Build tracks from event, add tracks in trackBuffer
    virtual void BuildTracks(STEvent*, std::vector<STLinearTrack*>*) {}
    virtual void BuildTracks(STEvent*, TClonesArray*) {}

  protected:
    FairLogger *fLogger;

    Double_t fNumTbs;

    Double_t fXUnit; ///< Unit length in x = pad width in x (mm)
    Double_t fYUnit; ///< Unit length in y = 1 time bucket length (mm)
    Double_t fZUnit; ///< Unit length in z = pad width in z (mm)

    /// Minimum number of hits-in-track for track to survive as final track
    Int_t fNumHitsTrackCut;
    /// Minimum number of hits-in-track for fitting line
    Int_t fNumHitsFit;
    /// Number of selected hits-in-track to correlate proximity with hit
    Int_t fNumHitsCompare;
    /// Maximum number of selected hits-in-track to correlate proximity with hit
    Int_t fNumHitsCompareMax;

    Double_t fProxXCut; ///< Cut for hit1 to hit2 distance in X
    Double_t fProxYCut; ///< Cut for hit1 to hit2 distance in Y
    Double_t fProxZCut; ///< Cut for hit1 to hit2 distance in Z
    Double_t fProxRCut; ///< Cut for hit1 to hit2 distance in XZ plane

    Double_t fProxLineCut;
    Double_t fProxPlaneCut;

    Double_t fRMSLineCut;   ///< Cut for RMS in track-line  fitting
    Double_t fRMSPlaneCut;  ///< Cut for RMS in track-plane fitting
    Double_t fRMSTrackCut;  ///< = Sqrt(planeCut*planeCut + lineCut*lineCut);

    Double_t fDirectionDotCut;
    Double_t fNormalDotCut;

    Double_t fRadialCut;
    Double_t fNumHitsVanishCut;

    TVector3 fVertex;

    Int_t fNumHitsAtHead;
  

  ClassDef(STVTrackFinder, 3)
};

#endif
