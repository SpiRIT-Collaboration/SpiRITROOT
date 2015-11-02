#ifndef STLINEARTRACKFINDER
#define STLINEARTRACKFINDER

#include "STLinearTrackFinderAbstract.hh"
#include "STLinearTrackingConf.hh"

#include <vector>
#include "TMath.h"

typedef std::vector<STHit*>          vecHit_t;
typedef std::vector<STLinearTrack*>  vecTrk_t;
typedef std::vector<STCorrLinearTH*> vecCTH_t;
typedef std::vector<STCorrLinearTT*> vecCTT_t;

typedef std::vector<STLinearTrack*>::iterator iterTrk_t;

/**
 * @author JungWoo Lee
 */

class STLinearTrackFinder : public STLinearTrackFinderAbstract
{
  public:
    STLinearTrackFinder();
    ~STLinearTrackFinder() {}

    void BuildTracks(STEvent* event, vecTrk_t *tracks);

  private:
    STLinearTrackFitter* fFitter;

    vecHit_t *fHitQueue;
    vecTrk_t *fTrackQueue;
    vecTrk_t *fTrackBufferTemp;
    vecTrk_t *fTrackBufferFinal;

    vecCTH_t *fCorrTH;
    vecCTH_t *fCorrTH_largeAngle;
    vecCTH_t *fCorrTH_justPerp;
    vecCTT_t *fCorrTT;

    Double_t fStretchXZ;
    Double_t fStretchY;

  private:
    /**
     * Build track from hits. Push track into tracks.
     * If varialbe createNewTracks is false, hit with no matching track is
     * left in the buffer without being erased or create new track.
     */
    void Build(vecTrk_t *tracks, 
               vecHit_t *hits, 
               vecCTH_t *corrTH,
                 Bool_t createNewTracks = kTRUE);

    /**
     * Merge tracks
     */
    void Merge(vecTrk_t *tracks);

    /**
     * Select survived track from tracks, push to tracks2. 
     * Dead track returns hit to hits.
     * If varialble thetaCut is set, track with angle-theta 
     * bigger than thetaCut cannot survive.
     */
    void Select(vecTrk_t *tracks, 
                vecTrk_t *tracks2, 
                vecHit_t *hits, 
                Double_t thetaCut = -TMath::Pi());

    /** 
     * Sort hits of tracks in the buffer
     */
    void SortHits(vecTrk_t *tracks);

    /**
     * Return hit from tracks to hits
     */
    void ReturnHits(STLinearTrack *track, vecHit_t *hits);

  /*
  private:
    Double_t fDNumTbs;

    Int_t fDNumHitsTrackCut;
    Int_t fDNumHitsFit;
    Int_t fDNumHitsCompare;
    Int_t fDNumHitsCompareMax;

    Double_t fDProxXCut;
    Double_t fDProxYCut;
    Double_t fDProxZCut;
    Double_t fDProxRCut;

    Double_t fDProxLineCut;
    Double_t fDProxPlaneCut;

    Double_t fDRMSLineCut;
    Double_t fDRMSPlaneCut;
    Double_t fDRMSTrackCut;

    Double_t fDDirectionDotCut;
    Double_t fDNormalDotCut;

  private:
    void SetNumHitsCut(Int_t numHitsTrackCut,
                       Int_t numHitsFit,
                       Int_t numHitsCompare,
                       Int_t numHitsCompareMax);

    void SetProximityCutFactor(Double_t xConst, 
                               Double_t yConst,
                               Double_t zConst);

    void SetProximityTrackCutFactor(Double_t proxLine, 
                                    Double_t proxPlane);

    void SetProximityRCut(Double_t val);

    void SetRMSCut(Double_t rmsLineCut, 
                   Double_t rmsPlaneCut);

    void SetDotProductCut(Double_t directionDotCut, 
                          Double_t normalDotCut);
    */

  ClassDef(STLinearTrackFinder, 1)
};

#endif
