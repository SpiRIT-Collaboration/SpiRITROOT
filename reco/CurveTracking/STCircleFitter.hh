/**
 * @brief Circle Fitter
 *
 * @author JungWoo Lee
 *
 * @detail 
 *
 *   With i-th data position (xi, yi, zi) and weight wi,
 *   and circle center position (xc, yc) and radius r,
 *   minimize S = SUM wi*(r^2 - ((xi-xc)^2 + (yi-yc)^2))^2
 */

#ifndef STCIRCLEFITTER
#define STCIRCLEFITTER

#include <vector>
#include "Rtypes.h"
#include "STHit.hh"

class STCircleFitter
{
  public:
    STCircleFitter() {};
    ~STCircleFitter() {};

    /// This method assume circle lies on the xz plane.
    Bool_t FitData(std::vector<STHit*> *hitArray,
                   Double_t &xCenter, 
                   Double_t &zCenter, 
                   Double_t &radius,
                   Double_t &rms);

  private:
    ClassDef(STCircleFitter, 1)
};

#endif
