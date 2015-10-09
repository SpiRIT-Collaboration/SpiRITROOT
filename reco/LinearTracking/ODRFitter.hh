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

    void FitPlane(); ///< Fit plane
    void FitLine();  ///< Fit line

    TVector3 GetCentroid();   ///< Get centroid
    TVector3 GetNormal();     ///< Get normal vector for plane
    TVector3 GetDirection();  ///< Get direction vector for line

       Int_t GetNumPoints();  ///< Get number of points
    Double_t GetWeightSum();  ///< Get sum of weights
    Double_t GetRMS();        ///< Get RMS

  private:
    /// Solve eigen value equation
    void SolveEigenValueEquation();
    /// Choose which eigen value to use.
    void ChooseEigenValue(Int_t iEV);

  private:
       Int_t fNumPoints; ///< Number of point set
    Double_t fWeightSum; ///< Sum of weights
    Double_t fSumOfPC2;  ///< Sum of square of distance from point to the centroid

    TMatrixD fCentroid;  ///< Centroid position of the point set
    TVectorD fNormal;    ///< Normal vector of ODR plane, or vector of ODR line

    TMatrixD fMatrixA;      ///< Matrix A
    TVectorD fEigenValues;  ///< eigen values sorted by decending order
    TMatrixD fEigenVectors; ///< eigen vectors sorted by decending eigen value order

    Double_t fRMS; /// Root mean square of the fit


  ClassDef(ODRFitter, 1)
};

#endif
