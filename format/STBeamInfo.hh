#ifndef __STBEAMINFO_HH
#define __STBEAMINFO_HH
#include "TNamed.h"

class STBeamInfo : public TNamed
{
public:
  STBeamInfo();
  virtual ~STBeamInfo() {};

  double fBeamAoQ;
  double fBeamZ;
  double fRotationAngleA;
  double fRotationAngleB;
  double fRotationAngleTargetPlaneA;
  double fRotationAngleTargetPlaneB;
  double fProjX;
  double fProjY;
  double fBeamEnergy;
  double fBeamVelocity;

  ClassDef(STBeamInfo, 1);
};

#endif
