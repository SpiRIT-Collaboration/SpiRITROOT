#ifndef STHELIXTRACKFINDER
#define STHELIXTRACKFINDER

#include "STPadPlaneMap.hh"
#include "STHelixTrack.hh"
#include "STHelixTrackFitter.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "TClonesArray.h"
#include <vector>

typedef std::vector<STHit*>* vhit_t;

class STHelixTrackFinder
{
  public:
    STHelixTrackFinder();
    ~STHelixTrackFinder() {}

    void BuildTracks(TClonesArray *hitArray, TClonesArray *trackArray, TClonesArray *hitClusterArray);

    TVector3 FindVertex(TClonesArray *tracks, Int_t nIterations = 1);

    void SetClusteringOption(Int_t opt);

  private:
    /** 
     * Create new track with free hit from event map
     * return nullptr if there no more free hit.
    */
    STHelixTrack *NewTrack();

    /** 
     * Build track with 1st neighbor hits until 
     * it is good enough to fit in hexli (# of hits < 10)
    */
    bool TrackInitialization(STHelixTrack* track);

    /** 
     * Build track with 1st neighbor hits (continuous)
     * using fitted parameter
    */
    bool TrackContinuum(STHelixTrack* track);

    /** 
     * Build track from the found hits by extrapolation of the tracks
     * using fitted parameter
    */
    bool TrackExtrapolation(STHelixTrack* track);

    /**
     * Build track by extrapolating head or tail.
     * Check if the head is changed to tail or vice versa.
     */
    bool AutoBuildByExtrapolation(STHelixTrack *, bool &, Double_t &);

    /**
     * Build track from given position.
     * Check if the head is changed to tail or vice versa.
     */
    bool AutoBuildAtPosition(STHelixTrack *, TVector3, bool &, Double_t &, Double_t rScale = 1);

    /** 
     * Check quality of the track
     * Return false if it is not satisfied.
    */
    bool TrackQualityCheck(STHelixTrack* track);

    /** 
     * Build track by 
     * 1) confirming already added hit,
     *    - remove isolated hit.
     * 2) look if any of hits are share with other tracklets and
     * 3) determine main hits and candidate hits.
    */
    bool TrackConfirmation(STHelixTrack* track);

    /** 
     * ConfimHits
    */
    bool ConfirmHits(STHelixTrack* track, bool &tailToHead);

    /**
     * Build track by interpolating with length.
     * Check if the head is changed to tail or vice versa.
     */
    bool AutoBuildByInterpolation(STHelixTrack *, bool &, Double_t &, Double_t rScale = 1);

    /**
     * Clusterize Hits
     */
    bool HitClustering(STHelixTrack *track, Double_t cut);
    bool HitClustering2(STHelixTrack *track);

    /**
     * Create new cluster with given hit.
    */
    STHitCluster *NewCluster(STHit *hit);

    /**
     * Check if hit is used.
     * Return, 
     *        -2 : free
     *        -1 : used but not own to track
     *  positive : track id of last owner
     */
    Int_t CheckHitOwner(STHit *hit);

    /**
     * Correlate hit-track by comparing width and height proximity
     */
    Double_t Correlate(STHelixTrack *track, STHit *hit, Double_t rScale = 1);

    /**
     * Correlate hit-track by checking hit is right next to hits from track.
     */
    Double_t CorrelateSimple(STHelixTrack *track, STHit *hit);

    /**
     * Calculate tangent of dipangle if asumming that track containing
     * hit comes from the target position.
     */
    Double_t TangentOfMaxDipAngle(STHit *hit);


  private:
    TClonesArray *fTrackArray = nullptr;       ///< STHelixTrack array
    TClonesArray *fHitClusterArray = nullptr;  ///< STHitCluster array
    STPadPlaneMap *fEventMap = nullptr;        ///< hit map to pad plane
    STHelixTrackFitter *fFitter = nullptr;     ///< Helix track fitter

    vhit_t fCandHits = nullptr;  ///< Candidate hits comming from fEventMap
    vhit_t fGoodHits = nullptr;  ///< Selected good hits from current fCandHits
    vhit_t fBadHits  = nullptr;  ///< Selected bad  hits from current fCandHits

    Int_t fClusteringOption = 0;


  ClassDef(STHelixTrackFinder, 2)
};

#endif
