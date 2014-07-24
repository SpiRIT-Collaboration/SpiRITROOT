//-----------------------------------------------------------
// Description:
//      Track finder using the riemann circle fit
//
//      The algorithm can be configured with correlators 
//      (see STAbsHitTrackCorrelator)
//      For each hit all track candidates are tested
//      For each track the algorithm is stepping through the 
//      correlators which are in a hirarchical order. 
//      A correlator can be applicable or not. 
//      If it is applicalble to the hit/track combination and
//      the track survives, then the correlator delivers a matchQuality.
//      Finally the hit is added to the track candidate that 
//      reached the deepest correlator level and 
//      if there are ambiguities achieved the best machtQuality
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Original Author List:
//      Sebastian Neubert    TUM            (original author)
//      Johannes Rauch       TUM
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------

#ifndef STRIEMANNTRACKFINDER_HH
#define STRIEMANNTRACKFINDER_HH

// STL
#include <vector>

using std::vector;

class STCluster;
class GFTrackCand;
class STAbsHitTrackCorrelator;
class STAbsTrackTrackCorrelator;
class STRiemannTrack;

class STRiemannTrackFinder {
  public:
    STRiemannTrackFinder();
    STRiemannTrackFinder(Double_t scale);
    ~STRiemannTrackFinder();


    // Accessors -----------------------
    const STAbsHitTrackCorrelator* GetHTCorrelator(UInt_t i) const
    {return fHTCorrelators.at(i);}
    const STAbsTrackTrackCorrelator* GetTTCorrelator(UInt_t i) const
    {return fTTcorrelators.at(i);}

    Int_t getSorting(){return fSorting;}
    Double_t getInteractionZ(){return fInteractionZ;}

    // Modifiers -----------------------
    void SetMinHitsForFit(UInt_t n){fMinHitsForFit=n;}

    void SetSorting(Int_t s){fSorting=s;} // -1: no sorting, 0: sort Clusters by X, 1: Y, 2: Z, 3: R, 4: Distance to interaction point, 5: Phi
    void SetSortingMode(Bool_t sortingMode){fSortingMode=sortingMode;} // false: sort only according to fSorting; kTRUE: use internal sorting when adding hits to trackcands
    void SetInteractionZ(Double_t z){fInteractionZ=z;}
    void SetMaxNumHitsForPR(Double_t MaxNumHitsForPR){fMaxNumHitsForPR=MaxNumHitsForPR;} // for debugging

    void SkipCrossingAreas(Bool_t opt=kTRUE) {fSkipCrossingAreas=opt;}
    void SetTTProxcut(Double_t cut){fTTproxcut=cut;} // needed for speeding up the merging
    void SetProxcut(Double_t cut){fProxcut=cut;} // needed for speeding up the initialized trackbuilding
    void SetHelixcut(Double_t cut){fHelixcut=cut;} // needed for excluding hits in crossing areas

    void initTracks(Bool_t initTrks=kTRUE, Double_t dip=0, Double_t curv=0) {fInitTrks=initTrks; fInitDip=dip; fInitCurv=curv;}
    void SetMaxR(Double_t R){fMaxR=R;}
    void SetMinHits(UInt_t i){fMinHits=i;}
    void SetSkipAndDelete(Bool_t opt=kTRUE) {fSkipAndDelete=opt;}

    void SetScale(Double_t scale){fRiemannScale=scale;}
    Double_t GetScale()const {return fRiemannScale;}

    // Operations ----------------------
    UInt_t buildTracks(vector<TpcCluster*>& clusters,
        vector<STRiemannTrack*>& candlist);

    void mergeTracks(vector<STRiemannTrack*>& candlist);
    void cleanTracks(vector<STRiemannTrack*>& candlist, Double_t szcut, Double_t planecut);

    void addCorrelator(STAbsHitTrackCorrelator* c);
    void addTTCorrelator(STAbsTrackTrackCorrelator* c);

    void sortClusters(vector<TpcCluster*>& cll);
    void sortTracklets(vector<STRiemannTrack*>& tracklets);

  private:
    // Private Data Members ------------
    vector<STAbsHitTrackCorrelator *> fHTCorrelators;
    vector<Bool_t> fFound; // flags which correlator fired
    vector<Double_t> fBestMatchQuality;
    vector<UInt_t> fBestMatchIndex;

    vector<STAbsTrackTrackCorrelator *> fTTCorrelators;

    Int_t fSorting;
    Bool_t fSortingMode;
    Double_t fInteractionZ;
    Int_t fMaxNumHitsForPR; // for debugging

    Bool_t fSkipCrossingAreas;

    Bool_t fInitTrks;
    Bool_t fSkipAndDelete; // skip hits far away from boundary and delete incomplete tracks!
    Double_t fInitDip;
    Double_t fInitCurv;
    Double_t fMaxR;
    UInt_t fMinHits;

    UInt_t fMinHitsForFit;

    Double_t fProxcut;
    Double_t fHelixcut;
    Double_t fTTproxcut;
    Double_t fRiemannScale;

    // Private Methods -----------------
    void InitVariables();
    void resetFlags();

};

// sorting algorithm for clusters
class sortClusterClass{
  public:
    Bool_t operator() (TpcCluster* s1, TpcCluster* s2);
    void SetSorting(Int_t s){sorting=s;}
    void SetInteractionZ(Double_t z){interactionZ=z;}
  private:
    Int_t sorting;
    Double_t interactionZ;
};

// sorting algorithm for tracklets
class sortTrackletsClass{
  public:
    Bool_t operator() (STRiemannTrack* t1, STRiemannTrack* t2);
    void SetSorting(Int_t s){sorting=s;}
  private:
    Int_t sorting;
};

#endif
