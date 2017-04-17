#ifndef STTRACKCANDIDATE_HH
#define STTRACKCANDIDATE_HH

#include "TObject.h"
#include "TVector3.h"
#include "STPID.hh"
#include "STdEdxPoint.hh"
#include "Track.h"

#include <vector>
using namespace std;

class STRecoTrackCand : public TObject
{
  protected:
    STPID::PID fPID; ///< STPID
    Double_t fPIDProbability; ///< PID probability

    TVector3 fMomentum; ///< momentum at vertex
    TVector3 fMomentumTargetPlane; ///< momentum at target plane
    TVector3 fPosTargetPlane; ///< position at target plane
    vector<STdEdxPoint> fdEdxPointArray; ///< Array of dEdxPoint

    genfit::Track *fGenfitTrack; //! < only used for task

  public:
    STRecoTrackCand();
    virtual ~STRecoTrackCand() {}

    virtual void Clear(Option_t *option = "");
    virtual void Print(Option_t *option = "") const;
    virtual void Copy(TObject *obj) const;

    void SetPID(STPID::PID val) { fPID = val; }
    STPID::PID GetPID() { return fPID; }

    void SetPIDProbability(Double_t val) { fPIDProbability = val; }
    Double_t GetPIDProbability() { return fPIDProbability; }

    void SetMomentum(TVector3 val) { fMomentum = val; }
    TVector3 GetMomentum() { return fMomentum; }

    void SetMomentumTargetPlane(TVector3 val) { fMomentumTargetPlane = val; }
    TVector3 GetMomentumTargetPlane() { return fMomentumTargetPlane; }

    void SetPosTargetPlane(TVector3 val) { fPosTargetPlane = val; }
    TVector3 GetPosTargetPlane() { return fPosTargetPlane; }

    void AdddEdxPoint(STdEdxPoint dedxPoint) { fdEdxPointArray.push_back(dedxPoint); }
    vector<STdEdxPoint> *GetdEdxPointArray() { return &fdEdxPointArray; }
    Double_t GetdEdxWithCut(Double_t lowCut, Double_t highCut);

    void SetGenfitTrack(genfit::Track *val) { fGenfitTrack = val; }
    genfit::Track *GetGenfitTrack() { return fGenfitTrack; }

  ClassDef(STRecoTrackCand, 1)
};

#endif
