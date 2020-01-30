#ifndef __STBEAMINFO_HH
#define __STBEAMINFO_HH
#include "TNamed.h"

class STBeamInfo : public TNamed
{
public:
  STBeamInfo();
  virtual ~STBeamInfo() {};

  double fBeamAoQ = 0;
  double fBeamZ = 0;
  double fRotationAngleATargetPlane = 0;
  double fRotationAngleBTargetPlane = 0;
  double fXTargetPlane = 0;
  double fYTargetPlane = 0;
  double fBeamEnergyTargetPlane = 0;
  double fBeamVelocityTargetPlane = 0;

  virtual void Clear(Option_t *option = "");

  ClassDef(STBeamInfo, 1);
};

#endif
