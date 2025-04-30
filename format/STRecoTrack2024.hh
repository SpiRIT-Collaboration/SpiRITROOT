#ifndef STRECOTRACK2024_HH
#define STRECOTRACK2024_HH

#include "TObject.h"
#include "TVector3.h"
#include "STRecoTrack.hh"
#include "STPID.hh"
#include "STdEdxPoint.hh"

#include <vector>
using namespace std;

/**
 * From STRecoTrack
*/
class STRecoTrack2024 : public STRecoTrack
{
  private:
    TVector3 fPosWindow; ///< position at extrapolation to window

  public:
    STRecoTrack2024();
    STRecoTrack2024(STRecoTrack2024 *recoTrack);
    virtual ~STRecoTrack2024() {}

    virtual void Clear(Option_t *option = "");

    void SetPosWindow(TVector3 val) { fPosWindow = val; }
    TVector3 GetPosWindow() { return fPosWindow; }

  ClassDef(STRecoTrack2024, 1);
};

#endif
