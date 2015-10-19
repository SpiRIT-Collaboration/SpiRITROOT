/**
 * @author JungWoo Lee
 */

#ifndef STLINEARTRACKFINDERABSTRACT
#define STLINEARTRACKFINDERABSTRACT

#include "STHit.hh"
#include "STEvent.hh"
#include "STLinearTrack.hh"
#include "STLinearTrackFitter.hh"

class STLinearTrackFinderAbstract
{
  public:
    STLinearTrackFinderAbstract();
    virtual ~STLinearTrackFinderAbstract() {} 

    /// Build tracks from event, add tracks in trackBuffer
    virtual void BuildTracks(STEvent*, std::vector<STLinearTrack*>*) = 0;

  protected:
    Double_t fNumTbs;

    Double_t fXUnit; ///< Unit length in x = pad width in x (mm)
    Double_t fYUnit; ///< Unit length in y = 1 time bucket length (mm)
    Double_t fZUnit; ///< Unit length in z = pad width in z (mm)

    /// Minimum number of hits-in-track for track to survive as final track
    Int_t fMinNumHitCut;
    /// Number of selected hits-in-track to correlate proximity with hit
    Int_t fNumHHCompare;
    /// Minimum number of hits-in-track for fitting line
    Int_t fMinNumHitFitLine;
    /// Minimum number of hits-in-track for fitting plane
    Int_t fMinNumHitFitPlane;

    Double_t fProxXCut; ///< Cut for hit1 to hit2 distance in X
    Double_t fProxYCut; ///< Cut for hit1 to hit2 distance in Y
    Double_t fProxZCut; ///< Cut for hit1 to hit2 distance in Z
    Double_t fProxRCut; ///< Cut for hit1 to hit2 distance in XZ plane

    Double_t fRMSLineCut;   ///< Cut for RMS in track-line  fitting
    Double_t fRMSPlaneCut;  ///< Cut for RMS in track-plane fitting
    Double_t fRMSTrackCut;  ///< = Sqrt(planeCut*planeCut + lineCut*lineCut);

    Double_t fDirectionDotCut;
    Double_t fNormalDotCut;
  
  public:
    /// Set number of hits cuts
    /// @param numHitsCut 
    ///   Minimum number of hits-in-track for track to survive as final track
    /// @param numHitsHHCompare 
    ///   Number of selected hits-in-track to correlate proximity with hit
    /// @param numHitsFitLine   
    ///   Minimum number of hits-in-track for fitting line
    /// @param numHitsFitPlane  
    ///   Minimum number of hits-in-track for fitting plane
    void SetNumHitsCut(Int_t numHitsCut,
                       Int_t numHitsHHCompare,
                       Int_t numHitsFitLine,
                       Int_t numHitsFitPlane);

    /// Set factor for proximity cuts
    /// @param xConst  x-proximity-cut = xConst * pad size x
    /// @param yConst  y-proximity-cut = xConst * 1 time-bucket size
    /// @param zConst  z-proximity-cut = zConst * pad size z
    void SetProximityCutFactor(Double_t xConst, 
                               Double_t yConst,
                               Double_t zConst);

    void SetProximityRCut(Double_t val);

    /// Set RMS cuts
    /// @param rmsLine   RMS cut for line
    /// @param rmsPlane  RMS cut for Plane
    void SetRMSCut(Double_t rmsLineCut, 
                   Double_t rmsPlaneCut);

    /// Set dot product cut of direction and normal vector of two tracks (merge) 
    /// @param directionDotCut  direction vector dot product cut for
    /// @param normalDotCut     norma vector dot product cut for
    void SetDotProductCut(Double_t directionDotCut, 
                          Double_t normalDotCut);

  ClassDef(STLinearTrackFinderAbstract, 1)
};

#endif
