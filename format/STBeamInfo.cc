#include "STBeamInfo.hh"

STBeamInfo::STBeamInfo(){}

void STBeamInfo::Clear(Option_t *option) {
  fBeamAoQ = 0;
  fBeamZ = 0;
  fRotationAngleATargetPlane = 0;
  fRotationAngleBTargetPlane = 0;
  fXTargetPlane = 0;
  fYTargetPlane = 0;
  fBeamEnergyTargetPlane = 0;
  fBeamVelocityTargetPlane = 0;
}
