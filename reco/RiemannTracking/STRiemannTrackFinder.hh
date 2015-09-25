#ifndef STRIEMANNTRACKFINDER_HH
#define STRIEMANNTRACKFINDER_HH

// STL
#include <vector>

using std::vector;

class STHit;
class STAbsHitTrackCorrelator;
class STAbsTrackTrackCorrelator;
class STRiemannTrack;

/**
 * @brief Track finder using the riemann circle fit
 *
 * @author Sebastian Neubert (TUM) -- original author
 * @author Johannes Rauch    (TUM)
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 *
 * @detail 
 *    The algorithm can be configured with correlators 
 *    (see STAbsHitTrackCorrelator)
 *    For each hit all track candidates are tested
 *    For each track the algorithm is stepping through the 
 *    correlators which are in a hirarchical order. 
 *    A correlator can be applicable or not. 
 *    If it is applicalble to the hit/track combination and
 *    track survives, then the correlator delivers a matchQuality.
 *    Finally the hit is added to the track candidate that 
 *    reached the deepest correlator level and 
 *    if there are ambiguities achieved the best machtQuality
 */

class STRiemannTrackFinder 
{
  public:
    STRiemannTrackFinder();
    STRiemannTrackFinder(Double_t scale);
    ~STRiemannTrackFinder();


    // Accessors -----------------------
    const STAbsHitTrackCorrelator *GetHTCorrelator(UInt_t i) const;
    const STAbsTrackTrackCorrelator *GetTTCorrelator(UInt_t i) const;

    Int_t GetSorting();
    Double_t GetInteractionZ();
    Double_t GetScale() const;

    // Modifiers -----------------------
    void SetMinHitsForFit(UInt_t numHits);

    /** 
     * @param sorting
     *        -1: no sorting, 
     *         0: sort Hits by X, 
     *         1: Y, 
     *         2: Z, 
     *         3: R, 
     *         4: Distance to interaction point, 
     *         5: Phi
     */
    void SetSorting(Int_t sorting);  
    /** 
     * false: sort only according to fSorting; 
     * true:  use internal sorting when adding hits to trackcands
     */
    void SetSortingMode(Bool_t sortingMode);
    void SetInteractionZ(Double_t z);
    void SetMaxNumHitsForPR(Double_t maxHits); ///< for debugging

    void SkipCrossingAreas(Bool_t value = kTRUE);
    void SetTTProxCut(Double_t cut); ///< needed for speeding up the merging
    void SetProxCut(Double_t cut);   ///< needed for speeding up the initialized trackbuilding
    void SetHelixCut(Double_t cut);  ///< needed for excluding hits in crossing areas

    void InitTracks(Bool_t initTracks = kTRUE, Double_t dip = 0, Double_t curv = 0);
    void SetMaxR(Double_t r);
    void SetMinHits(UInt_t minHits);
    void SetSkipAndDelete(Bool_t value = kTRUE);

    void SetScale(Double_t scale);

    // Operations ----------------------
    UInt_t BuildTracks(vector<STHit *> &hits, vector<STRiemannTrack *> &candlist);

    void MergeTracks(vector<STRiemannTrack *> &candlist);
    void CleanTracks(vector<STRiemannTrack *> &candlist, Double_t szcut, Double_t planecut);

    void AddHTCorrelator(STAbsHitTrackCorrelator *c);
    void AddTTCorrelator(STAbsTrackTrackCorrelator *c);

    void SortHits(vector<STHit *> &hitList);
    void SortTracklets(vector<STRiemannTrack *> &tracklets);

  private:
    // Private Data Members ------------
    vector<STAbsHitTrackCorrelator *> fHTCorrelators;
    vector<Bool_t> fFound; ///< flags which correlator fired
    vector<Double_t> fBestMatchQuality;
    vector<UInt_t> fBestMatchIndex;

    vector<STAbsTrackTrackCorrelator *> fTTCorrelators;

    Int_t fSorting;
    Bool_t fSortingMode;
    Double_t fInteractionZ;
    Int_t fMaxNumHitsForPR; ///< for debugging

    Bool_t fSkipCrossingAreas;

    Bool_t fInitTracks;
    Bool_t fSkipAndDelete; ///< skip hits far away from boundary and delete incomplete tracks!
    Double_t fInitDip;
    Double_t fInitCurv;
    Double_t fMaxR;
    UInt_t fMinHits;

    UInt_t fMinHitsForFit;

    Double_t fProxCut;
    Double_t fHelixCut;
    Double_t fTTProxCut;
    Double_t fRiemannScale;

    // Private Methods -----------------
    void InitVariables();
    void ResetFlags();

};

/// Sorting algorithm for hits
class SortHitClass
{
  public:
    Bool_t operator() (STHit *hit1, STHit *hit2);

    void SetSorting(Int_t sorting) { fSorting = sorting; }
    void SetInteractionZ(Double_t z) { fInteractionZ = z; }

  private:
    Int_t fSorting;
    Double_t fInteractionZ;
};

/// Sorting algorithm for tracklets
class SortTrackletsClass
{
  public:
    Bool_t operator() (STRiemannTrack *t1, STRiemannTrack *t2);

    void SetSorting(Int_t sorting) { fSorting = sorting; }

  private:
    Int_t fSorting;
};

#endif
