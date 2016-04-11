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

    /// This method Assume circle lies on the xz plane. (TPC convension)
    Bool_t FitData(std::vector<STHit*> *hitArray,
                   Double_t &xCenter, 
                   Double_t &zCenter, 
                   Double_t &radius,
                   Double_t &rms);

    /** 
     * General method.
     *
     * TVector3 in data array needs 3 components:
     * @param x first axis value
     * @param y second axis value
     * @param z weight of the point
     *
     * Set ouput parameters to:
     * @param xCenter first axis center of the fitted circle
     * @param yCenter second axis center of the fitted circle
     * @param radius  radius of the fitted circle
     * @param rms     root mean square of the fit
     */
    Bool_t Fit(std::vector<TVector3> *data,
               Double_t &xCenter, 
               Double_t &yCenter, 
               Double_t &radius,
               Double_t &rms);

  private:
    ClassDef(STCircleFitter, 1)
};

#endif
