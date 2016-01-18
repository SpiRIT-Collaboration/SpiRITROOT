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
      kNon         = -1,
      kMC          = 0,
      kDigi        = 1,
      kHit         = 2,
      kCluster     = 3,
      kClusterBox  = 4,
      kRiemannHit  = 5,
      kLinear      = 6,
      kLinearHit   = 7,
      kHitBox      = 8
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


  ClassDef(STEveTask, 1);
};

#endif
