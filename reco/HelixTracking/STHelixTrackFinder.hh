#ifndef STHELIXTRACKFINDER
#define STHELIXTRACKFINDER

#include "STPadPlaneMap.hh"
#include "STHelixTrack.hh"
#include "STHelixTrackFitter.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "TClonesArray.h"
#include "TMinuit.h"
#include <vector>

typedef std::vector<STHit*>* vhit_t;

class STHelixTrackFinder
{
  public:
    STHelixTrackFinder();
    ~STHelixTrackFinder() {}

    void BuildTracks(TClonesArray *hitArray, TClonesArray *trackArray, TClonesArray *hitClusterArray);

    TVector3 FindVertex(TClonesArray *tracks, Int_t nIterations = 1);

    /**
     * Scale = alpha x fDefaultScale; where alpha is 1 for survived
     * tracklets. Scale is multiplied to track cut.
     * Default is 2.5
    */
    void SetDefaultCutScale(Double_t scale);

    /**
     * If hit is with in the range of track width cut (in xz plane),
     * hit is satisfied with this cut to be added to the track.
     *
     * Cut value is limited in range
     * fTrackWCutLL < [cut_value] < fTrackWCutHL
     *
     * The [final cut value] is scaled with the scale factor.
     * [final cut value] = Scale x [cut_value] which means by default:
     * 2.5 x fTrackWCutLL < 2.5 x [cut_value] < 2.5 x fTrackWCutHL
    */
    void SetTrackWidthCutLimits(Double_t lowLimit, Double_t highLimit);

    /**
     * If hit is with in the range of track width cut (in xz plane),
     * hit is satisfied with this cut to be added to the track.
     *
     * Cut value is limited in range
     * fTrackWCutLL < [cut_value] < fTrackWCutHL
     *
     * The [final cut value] is scaled with the scale factor.
     * [final cut value] = Scale x [cut_value] which means by default:
     * 2.5 x fTrackWCutLL < 2.5 x [cut_value] < 2.5 x fTrackWCutHL
    */
    void SetTrackHeightCutLimits(Double_t lowLimit, Double_t highLimit);

    /**
     * Clusters at the four sides are flattened because of the partial charge at each side.
     * Those clusters drag tracks toward them making their momenta deviated from the true values.
     * Those clusters are disabled after they are found by the algorithm.
    */
    void SetClusterCutLRTB(Double_t left, Double_t right, Double_t top, Double_t bottom);

    /**
     * Hits are cut out by the cylinder or sphere
     * and clusters are removed by that cylinder or sphere + margin
     * since clusters are flattened along the cut surface.
    */
    void SetCylinderCut(TVector3 center, Double_t radius, Double_t zLength, Double_t margin);
    void SetSphereCut(TVector3 center, Double_t radius, Double_t margin);
    void SetEllipsoidCut(TVector3 center, TVector3 radii, Double_t margin);

    /**
     * Clustering direction changing angle. If margin is 0, then the direction changed at the "angle" value.
     * If margin is not 0, then the direction in unchanged until the angle is reached at "angle + margin".
     */
    void SetClusteringAngleAndMargin(Double_t angle, Double_t margin);

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
    bool HitClustering(STHelixTrack *track);
    bool HitClusteringMar4(STHelixTrack *helix);

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

    /**
     * Check if the cluster is created with continuous hits.
     */
    void CheckIsContinuousHits(STHitCluster *cluster);

    /**
     * Using the alpha angle, determine the clustering direction to be by layer or row.
     */
    Bool_t CheckBuildByLayer(STHelixTrack *helix, STHit *hit, STHit *prevHit = nullptr);

  TClonesArray *fTrackArray = nullptr;       ///< STHelixTrack array
  TClonesArray *fHitClusterArray = nullptr;  ///< STHitCluster array
  STPadPlaneMap *fEventMap = nullptr;        ///< hit map to pad plane used in clustering
  STPadPlaneMap *fHitMap   = nullptr;        ///< hit map used for finding saturated hits
  STHelixTrackFitter *fFitter = nullptr;     ///< Helix track fitter
  
  vhit_t fCandHits = nullptr;  ///< Candidate hits comming from fEventMap
  vhit_t fGoodHits = nullptr;  ///< Selected good hits from current fCandHits
    vhit_t fBadHits  = nullptr;  ///< Selected bad  hits from current fCandHits

    Double_t fDefaultScale = 2.5;
    Double_t fTrackWCutLL = 4.;  //< Track width cut low limit
    Double_t fTrackWCutHL = 10.; //< Track width cut high limit
    Double_t fTrackHCutLL = 2.;  //< Track height cut low limit
    Double_t fTrackHCutHL = 4.;  //< Track height cut high limit

    Double_t fCCLeft = 0;   //< Cluster cut left
    Double_t fCCRight = 0;  //< Cluster cut right
    Double_t fCCTop = 0;    //< Cluster cut top
    Double_t fCCBottom = 0; //< Cluster cut bottom

    TVector3 fCutCenter = TVector3(-9999, -9999, -9999); //< Cut center
    Double_t fCRadius = -1;       //< Radius of cylinder
    Double_t fZLength = -1;       //< Length of cylinder
    Double_t fSRadius = -1;       //< Radius of sphere
    TVector3 fERadii = TVector3(-1, -1, -1);  //< Radii of ellipsoid
    Double_t fCutMargin = -1;     //< Cut margin for cluster cut

    Double_t fClusteringAngle = 45.; //< [Deg]. Clustering direction changing angle from row to layer, and vice versa;
    Double_t fClusteringMargin = 0.; //< [Deg]. Margin of clustering direction changing angle.

  ClassDef(STHelixTrackFinder, 3)
};

#endif
