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
    Int_t fParentID; ///< ID from parent track
    Int_t fVertexID; ///< ID from vertex branch
    Int_t fHelixID; ///< Helix track ID

    TVector3 fPOCAVertex; ///< point of closest approach to vertex
    TVector3 fPosKyotoL; ///< position at extrapolation to left kyoto plane
    TVector3 fPosKyotoR; ///< position at extrapolation to right kyoto plane
    TVector3 fPosKatana; ///< position at extrapolation to katana plane
    TVector3 fPosNeuland; ///< position at extrapolation to NeuLAND plane

    vector<Int_t> fClusterIDArray; ///< Array of cluster-ID

    Double_t fEffCurvature1;
    Double_t fEffCurvature2;
    Double_t fEffCurvature3;

  public:
    STRecoTrack();
    STRecoTrack(STRecoTrack*  aTrack_tem); //Rensheng add
    virtual ~STRecoTrack() {}

    virtual void Clear(Option_t *option = "");

    void SetCharge(Int_t val) { fCharge = val; }
    Int_t GetCharge() { return fCharge; }

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

  ClassDef(STRecoTrack, 4);
};

#endif
