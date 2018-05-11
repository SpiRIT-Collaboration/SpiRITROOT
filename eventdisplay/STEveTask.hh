#ifndef STEVETAKSK
#define STEVETAKSK

#include "FairTask.h"

class STEveTask : public FairTask
{
  public:
    STEveTask() {}
    STEveTask(TString name) : FairTask(name) {}
    ~STEveTask() {}

    enum STEveObject
    {
      kNon          = -1,
      kMC           = 0,
      kDigi         = 1,
      kHit          = 2,
      kCluster      = 3,
      kClusterBox   = 4,
      kRiemannTrack = 5,
      kRiemannHit   = 6,
      kHelix        = 7,
      kHelixHit     = 8,
      kCurve        = 9,
      kCurveHit     = 10,
      kHitBox       = 11,
      kRecoTrack    = 12,
      kRecoVertex   = 13
    };

    STEveObject GetEveObject(TString name);

    virtual void DrawADCTask(Int_t row, Int_t layer);
    virtual void DrawADC(Int_t row, Int_t layer) {}

    virtual void UpdateWindowTbTask(Int_t start, Int_t end);
    virtual void UpdateWindowTb(Int_t start, Int_t end) {}

    virtual void PushParametersTask();
    virtual void PushParameters() {}

    virtual Int_t RnrEveObjectTask(TString name, Int_t option = 1);
    virtual Int_t RnrEveObject(TString name, Int_t option) { return -1; }

    virtual Int_t IsSetTask(TString name, Int_t option = 1);
    virtual Int_t IsSet(TString name, Int_t option) { return -1; }

  protected:
    static const Int_t fNumEveObject = 15;


  ClassDef(STEveTask, 2);
};

#endif
