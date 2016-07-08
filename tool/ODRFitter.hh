/**
 * @brief Orthogonal Distance Regression (ODR) Fitter.
 *
 * @author JungWoo Lee
 *
 * @detail 
 *
 *   Orthogonal distance regression plane is found by minimizing
 *   f(X) (sum of perpendicular distance from point to the plane). 
 *   The process is done by solving the eigen value equation of 
 *   matrix M that is made from f(X). The ODR plane choosen with
 *   the smallest eigen value.
 *
 *   Orthogonal distance regression line is found by minimizing 
 *   the sum of perpendicular distance from point to the line. 
 *   The ODR line is choosen with the largest eigen value of matrix M.
 */

#ifndef ODRFITTER
#define ODRFITTER

#include "TVector3.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TMath.h"
#include <iostream>

class ODRFitter
{
  public:
    ODRFitter();
    ~ODRFitter();

    void Reset(); ///< Reset fitter

    void SetCentroid(Double_t x, Double_t y, Double_t z); ///< Set centroid 
    void SetCentroid(TVector3 position); ///< Set centroid with TVector3

    /** 
     *  @brief  Add point as a fitting data
     *
     *  @param x  x-position of the point point
     *  @param y  y-position of the point point
     *  @param z  z-position of the point point
     *  @param w  weight of the point point
     * 
     *  @detail 
     *    Before calling this method, one needs to set centroid 
     *    of the points with SetCentroid() methods. This method 
     *    will calculate the matrix A and sum of square of 
     *    distance from point to the centroid for fitting line.
     */
    void AddPoint(Double_t x, Double_t y, Double_t z, Double_t w = 1);

    void SetMatrixA(Double_t c00, 
                    Double_t c01, 
                    Double_t c02,
                    Double_t c11, 
                    Double_t c12, 
                    Double_t c22);
    void SetWeightSum(Double_t weightSum);
    void SetNumPoints(Double_t numPoints);

    bool FitPlane(); ///< Fit plane
    bool FitLine();  ///< Fit line

    /// Solve eigen value equation
    bool Solve();
    /// Choose which eigen value to use.
    void ChooseEigenValue(Int_t iEV);

    TVector3 GetCentroid();   ///< Get centroid
    TVector3 GetNormal();     ///< Get normal vector for plane
    TVector3 GetDirection();  ///< Get direction vector for line

       Int_t GetNumPoints();  ///< Get number of points
    Double_t GetWeightSum();  ///< Get sum of weights
    Double_t GetRMSLine();    ///< Get RMS of the line fit 
    Double_t GetRMSPlane();   ///< Get RMS of the plane fit 

  private:
       Int_t fNumPoints; ///< Number of point set
    Double_t fWeightSum; ///< Sum of weights
    Double_t fSumOfPC2;  ///< Sum of square of distance from point to the centroid

    Double_t fXCentroid;
    Double_t fYCentroid;
    Double_t fZCentroid;
    //TMatrixD *fCentroid;  ///< Centroid position of the point set
    TVectorD *fNormal;    ///< Normal vector of ODR plane, or vector of ODR line

    TMatrixD *fMatrixA;      ///< Matrix A
    TVectorD *fEigenValues;  ///< eigen values sorted by decending order
    TMatrixD *fEigenVectors; ///< eigen vectors sorted by decending eigen value order

    Double_t fRMSLine; /// Root mean square of the line fit
    Double_t fRMSPlane; /// Root mean square of the plane fit


  ClassDef(ODRFitter, 2)
};

#endif
