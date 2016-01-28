#ifndef STLINEARTRACKFINDER
#define STLINEARTRACKFINDER

#include "STVTrackFinder.hh"
#include "STLinearTrackingConf.hh"
#include "TClonesArray.h"

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

class STLinearTrackFinder : public STVTrackFinder
{
  public:
    STLinearTrackFinder();
    ~STLinearTrackFinder() {}

    void BuildTracks(STEvent* event, vecTrk_t *trackArray);

  private:
    STLinearTrackFitter* fFitter;

    vecHit_t *fHitQueue;
    vecTrk_t *fTrackQueue;
    vecTrk_t *fTrackBufferTemp;
    vecTrk_t *fTrackBufferFinal;
    TClonesArray *fTrackClonesArray;

    vecCTH_t *fCorrTH;
    vecCTH_t *fCorrTH_largeAngle;
    vecCTH_t *fCorrTH_largeAngle2;
    vecCTH_t *fCorrTH_justPerp;
    vecCTT_t *fCorrTT;

    Double_t fStretchXZ;
    Double_t fStretchY;

    Int_t fNumTracks; ///< Number of created tracks ever.

  private:
    /**
     * Build track from hitArray. Push track into trackArray.
     * If varialbe createNewTracks is false, hit with no matching track is
     * left in the buffer without being erased or create new track.
     */
    void Build(vecTrk_t *trackArray, 
               vecHit_t *hitArray, 
               vecCTH_t *corrTH,
                 Bool_t createNewTracks = kTRUE);

    /**
     * Merge trackArray
     */
    void Merge(vecTrk_t *trackArray);

    /**
     * Select survived track from trackArray, push to trackArray2. 
     * Dead track returns hit to hitArray.
     * If varialble thetaCut is set, track with angle-theta 
     * bigger than thetaCut cannot survive.
     */
    void Select(vecTrk_t *trackArray, 
                vecTrk_t *trackArray2, 
                vecHit_t *hitArray, 
                Double_t thetaCut = -TMath::Pi());

    /** 
     * Sort hitArray of trackArray in the buffer
     */
    void SortHits(vecTrk_t *trackArray);

    /**
     * Return hit from trackArray to hitArray
     */
    void ReturnHits(STLinearTrack *track, vecHit_t *hitArray);

  ClassDef(STLinearTrackFinder, 1)
};

#endif
