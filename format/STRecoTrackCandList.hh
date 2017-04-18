#ifndef STRECOTRACKCANDLIST_HH
#define STRECOTRACKCANDLIST_HH

#include "TObjArray.h"
#include "STRecoTrackCand.hh"
#include "STPID.hh"
#include <vector>
using namespace std;

class STRecoTrackCandList : public TObjArray
{
  private:
    vector<Int_t> fHitIDArray;
    STPID::PID fBestPID;

  public:
    STRecoTrackCandList();
    virtual ~STRecoTrackCandList() {}

    virtual void Clear(Option_t *option = "");
    virtual void Print(Option_t *option = "") const;

    STRecoTrackCand *GetRecoTrackCand(Int_t pid);

    void AddHitID(Int_t val) { fHitIDArray.push_back(val); }
    vector<Int_t> *GetHitIDArray() { return &fHitIDArray; }

    void SetBestPID(STPID::PID val) { fBestPID = val; }
    STPID::PID GetBestPID() { return fBestPID; }

  ClassDef(STRecoTrackCandList, 1)
};

#endif
