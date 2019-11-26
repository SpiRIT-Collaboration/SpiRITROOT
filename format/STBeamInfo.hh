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
  double fRotationAngleA = 0;
  double fRotationAngleB = 0;
  double fRotationAngleTargetPlaneA = 0;
  double fRotationAngleTargetPlaneB = 0;
  double fProjX = 0;
  double fProjY = 0;
  double fBeamEnergy = 0;
  double fBeamEnergyTargetPlane = 0;
  double fBeamVelocity = 0;
  double fBeamVelocityTargetPlane = 0;

  ClassDef(STBeamInfo, 1);
};

#endif
