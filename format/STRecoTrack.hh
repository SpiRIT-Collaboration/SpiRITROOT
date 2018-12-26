#ifndef STRECOTRACK_HH
#define STRECOTRACK_HH

#include "TObject.h"
#include "TVector3.h"
#include "STRecoTrackCand.hh"
#include "STPID.hh"
#include "STdEdxPoint.hh"

#include <vector>
using namespace std;

/**
 * From STRecoTrackCand
 *   STPID::PID fPID;
 *   Double_t fPIDProbability;
 *   TVector3 fMomentum;
 *   TVector3 fPosTargetPlane;
 *   vector<STdEdxPoint> fdEdxPointArray;
*/
class STRecoTrack : public STRecoTrackCand
{
  private:
    Int_t fCharge; ///< Charge of the particle
    Int_t fGFCharge; ///< Charge of the particle by GenFit
    Int_t fParentID; ///< ID from parent track
    Int_t fVertexID; ///< ID from vertex branch
    Int_t fHelixID; ///< Helix track ID

    TVector3 fPOCAVertex; ///< point of closest approach to vertex
    TVector3 fPosKyotoL; ///< position at extrapolation to left kyoto plane
    TVector3 fPosKyotoR; ///< position at extrapolation to right kyoto plane
    TVector3 fPosKatana; ///< position at extrapolation to katana plane
    TVector3 fPosNeuland; ///< position at extrapolation to NeuLAND plane

    //if cluster is in the layer or row corresponding to the vector index position a 1 is given else 0
    //separate array for byrow and bylayer
    std::vector<Int_t> fByLayerClusters;
    std::vector<Int_t> fByRowClusters;
    vector<Int_t> fClusterIDArray; ///< Array of cluster-ID

    Double_t fEffCurvature1;
    Double_t fEffCurvature2;
    Double_t fEffCurvature3;

    Int_t fNumRowClusters;   ///< The number of clusters from helix tracking
    Int_t fNumLayerClusters;   ///< The number of clusters from helix tracking
    Int_t fNumRowClusters90; ///< The number of clusters from helix tracking up to the layer 90
    Int_t fNumLayerClusters90; ///< The number of clusters from helix tracking up to the layer 90

    Double_t fHelixChi2R;    ///< Chi2 value with the helix track with respect to the perpendicular direction to the track
    Double_t fHelixChi2X;    ///< Chi2 value with the helix track with respect to the wire direction
    Double_t fHelixChi2Y;    ///< Chi2 value with the helix track with respect to the drift direction
    Double_t fHelixChi2Z;    ///< Chi2 value with the helix track with respect to the beam direction

    Double_t  fTrackLength;   ///< Track length from GenFit
    Int_t fNDF;               ///< The number of degrees of freedom from GenFit
    Double_t fChi2;           ///< Chi2 value from GenFit
    Double_t fChi2R;          ///< Chi2 value from GenFit with respect to the perpendicular direction to the track
    Double_t fChi2X;          ///< Chi2 value with GenFit track with respect to the wire direction 
    Double_t fChi2Y;          ///< Chi2 value with GenFit track with respect to the drift direction
    Double_t fChi2Z;          ///< Chi2 value with GenFit track with respect to the beam direction
    Int_t fRecoID;            ///< ID of STRecoTrack for vertex-added refit track. So, STRecoTrack always have -1.

  public:
    STRecoTrack();
    STRecoTrack(STRecoTrack *recoTrack);
    virtual ~STRecoTrack() {}

    virtual void Clear(Option_t *option = "");

    void SetCharge(Int_t val) { fCharge = val; }
    Int_t GetCharge() { return fCharge; }

    void SetGenfitCharge(Int_t val) { fGFCharge = val; }
    Int_t GetGenfitCharge() { return fGFCharge; }

    void SetParentID(Int_t val) { fParentID = val; }
    Int_t GetParentID() { return fParentID; }

    void SetVertexID(Int_t val) { fVertexID = val; }
    Int_t GetVertexID() { return fVertexID; }

    void SetHelixID(Int_t val) { fHelixID = val; }
    Int_t GetHelixID() { return fHelixID; }

    void SetPOCAVertex(TVector3 val) { fPOCAVertex = val; }
    TVector3 GetPOCAVertex() { return fPOCAVertex; }

    void SetPosKyotoL(TVector3 val) { fPosKyotoL = val; }
    TVector3 GetPosKyotoL() { return fPosKyotoL; }

    void SetPosKyotoR(TVector3 val) { fPosKyotoR = val; }
    TVector3 GetPosKyotoR() { return fPosKyotoR; }

    void SetPosKatana(TVector3 val) { fPosKatana = val; }
    TVector3 GetPosKatana() { return fPosKatana; }

    void SetPosNeuland(TVector3 val) { fPosNeuland = val; }
    TVector3 GetPosNeuland() { return fPosNeuland; }

    void AddClusterID(Int_t id) { fClusterIDArray.push_back(id); }
    vector<Int_t> *GetClusterIDArray() { return &fClusterIDArray; }

    void SetEffCurvature1(Double_t val) { fEffCurvature1 = val; }
    void SetEffCurvature2(Double_t val) { fEffCurvature2 = val; }
    void SetEffCurvature3(Double_t val) { fEffCurvature3 = val; }
    Double_t GetEffCurvature1() { return fEffCurvature1; }
    Double_t GetEffCurvature2() { return fEffCurvature2; }
    Double_t GetEffCurvature3() { return fEffCurvature3; }

    void SetRowVec(std::vector<Int_t> val) { fByRowClusters = val; }
    void SetLayerVec(std::vector<Int_t> val) { fByLayerClusters = val; }
    
    std::vector<Int_t>  GetRowVec() { return fByRowClusters; }
    std::vector<Int_t>  GetLayerVec() { return fByLayerClusters; }
  
  
    void SetNumRowClusters(Int_t val) { fNumRowClusters = val; }
    Int_t GetNumRowClusters() { return fNumRowClusters; }

    void SetNumLayerClusters(Int_t val) { fNumLayerClusters = val; }
    Int_t GetNumLayerClusters() { return fNumLayerClusters; }

    void SetNumRowClusters90(Int_t val) { fNumRowClusters90 = val; }
    Int_t GetNumRowClusters90() { return fNumRowClusters90; }

    void SetNumLayerClusters90(Int_t val) { fNumLayerClusters90 = val; }
    Int_t GetNumLayerClusters90() { return fNumLayerClusters90; }

    void SetHelixChi2R(Double_t val) { fHelixChi2R = val; }
    Double_t GetHelixChi2R() { return fHelixChi2R; }

    void SetHelixChi2X(Double_t val) { fHelixChi2X = val; }
    Double_t GetHelixChi2X() { return fHelixChi2X; }

    void SetHelixChi2Y(Double_t val) { fHelixChi2Y = val; }
    Double_t GetHelixChi2Y() { return fHelixChi2Y; }

    void SetHelixChi2Z(Double_t val) { fHelixChi2Z = val; }
    Double_t GetHelixChi2Z() { return fHelixChi2Z; }

    void SetTrackLength(Double_t val) { fTrackLength = val; }
    Double_t GetTrackLength() { return fTrackLength; }

    void SetNDF(Int_t val) { fNDF = val; }
    Int_t GetNDF() { return fNDF; }

    void SetChi2(Double_t val) { fChi2 = val; }
    Double_t GetChi2() { return fChi2; }

    void SetChi2R(Double_t val) { fChi2R = val; }
    Double_t GetChi2R() { return fChi2R; }

    void SetChi2X(Double_t val) { fChi2X = val; }
    Double_t GetChi2X() { return fChi2X; }

    void SetChi2Y(Double_t val) { fChi2Y = val; }
    Double_t GetChi2Y() { return fChi2Y; }

    void SetChi2Z(Double_t val) { fChi2Z = val; }
    Double_t GetChi2Z() { return fChi2Z; }

    void SetRecoID(Int_t val) { fRecoID = val; }
    Int_t GetRecoID() { return fRecoID; }

  ClassDef(STRecoTrack, 8);
};

#endif
