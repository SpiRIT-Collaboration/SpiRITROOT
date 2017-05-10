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
    Int_t fVertexID; ///< ID from vertex branch

    TVector3 fPOCAVertex; ///< point of closest approach to vertex
    TVector3 fPosKyotoL; ///< position at extrapolation to left kyoto plane
    TVector3 fPosKyotoR; ///< position at extrapolation to right kyoto plane
    TVector3 fPosKatana; ///< position at extrapolation to katana plane

    vector<Int_t> fClusterIDArray; ///< Array of cluster-ID

  public:
    STRecoTrack();
    virtual ~STRecoTrack() {}

    virtual void Clear(Option_t *option = "");

    void SetCharge(Int_t val) { fCharge = val; }
    Int_t GetCharge() { return fCharge; }

    void SetVertexID(Int_t val) { fVertexID = val; }
    Int_t GetVertexID() { return fVertexID; }

    void SetPOCAVertex(TVector3 val) { fPOCAVertex = val; }
    TVector3 GetPOCAVertex() { return fPOCAVertex; }

    void SetPosKyotoL(TVector3 val) { fPosKyotoL = val; }
    TVector3 GetPosKyotoL() { return fPosKyotoL; }

    void SetPosKyotoR(TVector3 val) { fPosKyotoR = val; }
    TVector3 GetPosKyotoR() { return fPosKyotoR; }

    void SetPosKatana(TVector3 val) { fPosKatana = val; }
    TVector3 GetPosKatana() { return fPosKatana; }

    void AddClusterID(Int_t id) { fClusterIDArray.push_back(id); }
    vector<Int_t> *GetClusterIDArray() { return &fClusterIDArray; }

  ClassDef(STRecoTrack, 2);
};

#endif
