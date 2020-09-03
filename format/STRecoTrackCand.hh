#ifndef STTRACKCANDIDATE_HH
#define STTRACKCANDIDATE_HH

#include "TObject.h"
#include "TVector3.h"
#include "STPID.hh"
#include "STdEdxPoint.hh"
#include "STHelixTrack.hh"
#include "Track.h"

#include <vector>
using namespace std;

class STRecoTrackCand : public TObject
{
  protected:
    Bool_t fIsEmbed; /// True if embeded clusters exist
    Int_t num_embed_clusters = -99;
    STPID::PID fPID; ///< STPID
    Double_t fPIDProbability; ///< PID probability

    TVector3 fMomentum; ///< momentum at vertex
    TVector3 fMomentumTargetPlane; ///< momentum at target plane
    TVector3 fPriMomentum; ///< momentum with primary pos
    TVector3 fPosTargetPlane; ///< position at target plane
    TVector3 fPriPosTargetPlane; ///< position at target plane
    TVector3 fPosVtxCluster; ///< position at target plane
    vector<STdEdxPoint> fdEdxPointArray; ///< Array of dEdxPoint

    genfit::Track *fGenfitTrack; //! < only used for task
    STHelixTrack *fHelixTrack; //! < only used for task

  public:
    STRecoTrackCand();
    virtual ~STRecoTrackCand() {}

    virtual void Clear(Option_t *option = "");
    virtual void Print(Option_t *option = "") const;
    virtual void Copy(TObject *obj) const;

    void SetIsEmbed(Bool_t val) { fIsEmbed = val; }
    void SetPID(STPID::PID val) { fPID = val; }

    Int_t GetNumEmbedClusters() {return num_embed_clusters; }
    void SetNumEmbedClusters(Int_t numCl) {num_embed_clusters = numCl; }
    STPID::PID GetPID() { return fPID; }

    void SetPIDProbability(Double_t val) { fPIDProbability = val; }
    Double_t GetPIDProbability() { return fPIDProbability; }

    void SetMomentum(TVector3 val) { fMomentum = val; }
    TVector3 GetMomentum() { return fMomentum; }

    void SetMomentumTargetPlane(TVector3 val) { fMomentumTargetPlane = val; }
    TVector3 GetMomentumTargetPlane() { return fMomentumTargetPlane; }

    void SetPriMomentum(TVector3 val) { fPriMomentum = val; }
    TVector3 GetPriMomentum() { return fPriMomentum; }

    void SetPosTargetPlane(TVector3 val) { fPosTargetPlane = val; }
    TVector3 GetPosTargetPlane() { return fPosTargetPlane; }

    void SetPriPosTargetPlane(TVector3 val) { fPriPosTargetPlane = val; }
    TVector3 GetPriPosTargetPlane() { return fPriPosTargetPlane; }

    void SetPosVtxCluster(TVector3 val) { fPosVtxCluster = val; }
    TVector3 GetPosVtxCluster() { return fPosVtxCluster; }

    void AdddEdxPoint(STdEdxPoint dedxPoint) { fdEdxPointArray.push_back(dedxPoint); }
    vector<STdEdxPoint> *GetdEdxPointArray() { return &fdEdxPointArray; }

    /*
     *
     *       fract_shadow: 1. includes all clusters when calculating dE/dx clusters with value < fract_shadow are included in dE/dx calculation 
     *    isContinuousHit: If true, dE/dx values are calculated only with the clusters having continuos hit distribution.
     *                     For example, the cluster created by layer and having hit distribution with row values 14, 15, 16, 17 is used
     *                     while the cluster with row values 14, 15, 17 is not used in the calculation of dE/dx.
     *        clusterSize: If 0, all clusters which haven't cut out by the other cuts are used in dE/dx calculation.
     *                     If non 0, clusters having hits more than that number are used in the calculation.
     *                     For example, with the value 2, clusters having 2 and less than 2 are not used in the calculation.
     *     numDiv, refDiv: The number of clusters is divided by numDiv and pick refDiv division as a reference value.
     *                     For example, with 20 clusters, numDiv=4 and refDiv=3, clusters are divided into [0,5), [5,10), [10,15) and [15,20)
     *                     and mean value of the third piece, [10,15), is used as a reference value, M([10,15)).
     *     cutMin, cutMax: These values are TMath::Log2(ratio).
     *                     For example with the same condition as above, if cutMin < TMath::Log2(M[0,5)/M[10,15)) < cutMax,
     *                     then the clusters in [0,5) are used to calculate dE/dx value.
     */
    Double_t GetdEdxWithCut(Double_t lowCut, Double_t highCut,
                            Double_t fract_shadow = 1.,
                            Bool_t isContinuousHits = kFALSE,
                            Int_t clusterSize = 0,
                            Int_t numDiv = -1, Int_t refDiv = -1,Double_t cutMin = -0.5, Double_t cutMax = 0.5);
    Int_t GetClustNum(Double_t fract_shadow = 1.);

    void SetGenfitTrack(genfit::Track *val) { fGenfitTrack = val; }
    genfit::Track *GetGenfitTrack() { return fGenfitTrack; }

  void SetHelixTrack(STHelixTrack *val)
  {
    fHelixTrack = val;
    fIsEmbed = fHelixTrack -> IsEmbed();
    //    num_embed_clusters = fHelixTrack -> GetNumEmbedClusters();
  }
    STHelixTrack *GetHelixTrack() { return fHelixTrack; }

    Bool_t IsEmbed(){return fIsEmbed; }

  ClassDef(STRecoTrackCand, 3)
};

#endif
