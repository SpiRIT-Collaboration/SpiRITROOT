/**
 * @brief Riemann Fitter
 *
 * @author JungWoo Lee
 *
 * @detail Fit using riemann sphere projection and ODR fit.
 */

#ifndef STRIEMANNFITTER
#define STRIEMANNFITTER

#include <vector>
#include "Rtypes.h"
#include "STHit.hh"
#include "ODRFitter.hh"

class STRiemannFitter
{
  public:
    STRiemannFitter();
    ~STRiemannFitter() {};

    /// This method assume circle lies on the xz plane.
    Bool_t FitData(std::vector<STHit*> *hitArray,
                   Double_t &xCenter, 
                   Double_t &zCenter, 
                   Double_t &radius,
                   Double_t &rms,
                   Double_t &rmsP);

    /** 
     * General method.
     *
     * TVector3 in data array needs 3 components:
     * @param x first axis value
     * @param y second axis value
     * @param z weight of the point
     *
     * Set ouput parameters to:
     * @param xCenter  first axis center of the fitted circle
     * @param yCenter  second axis center of the fitted circle
     * @param radius   radius of the fitted circle
     * @param rms      root mean square of the circle fit
     * @param rmsPlane root mean square of the plane fit on riemann sphere
     */
    Bool_t Fit(std::vector<TVector3> *data,
               Double_t &xCenter, 
               Double_t &yCenter, 
               Double_t &radius,
               Double_t &rms,
               Double_t &rmsPlane);

  private:
    ODRFitter *fODRFitter;

  ClassDef(STRiemannFitter, 1)
};

#endif
