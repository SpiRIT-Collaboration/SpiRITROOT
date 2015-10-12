/// @brief Orthogonal Distance Regression (ODR) Fitter.
/// @author JungWoo Lee

#include "ODRFitter.hh"

ClassImp(ODRFitter)

ODRFitter::ODRFitter()
{
  fCentroid = new TMatrixD(3,1);
  fNormal = new TVectorD(3);

  fMatrixA = new TMatrixD(3,3);
  fEigenValues = new TVectorD(3);
  fEigenVectors = new TMatrixD(3,3);

  Reset();
}

ODRFitter::~ODRFitter()
{
}

void ODRFitter::Reset()
{
  fNumPoints = 0;
  fWeightSum = 0;
  fSumOfPC2 = 0;

  for (Int_t i = 0; i < 3; i++) {
    for (Int_t j = 0; j < 3; j++) {
      (*fMatrixA)[i][j] = 0;
    }
  }

  fRMS = -1;
}

void ODRFitter::SetCentroid(Double_t x, Double_t y, Double_t z)
{
  (*fCentroid)[0][0] = x;
  (*fCentroid)[1][0] = y;
  (*fCentroid)[2][0] = z;
}

void ODRFitter::AddPoint(Double_t x, Double_t y, Double_t z, Double_t w)
{
  // building position matrix
  TMatrixD matrixP(3, 1);
  matrixP[0][0] = x;
  matrixP[1][0] = y;
  matrixP[2][0] = z;
  
  // building difference from position to centroid matrix
  TMatrixD matrixD(3, 1); 
  matrixD = matrixP - (*fCentroid);

  // adding to matrix A with position matrix and difference matrix
  TMatrixD matrixDT(TMatrixD::kTransposed, matrixD);
  TMatrixD matrixD2(matrixD, TMatrixD::kMult, matrixDT);
  matrixD2 *= w;
  (*fMatrixA) += matrixD2;  

  // adding to the square of distance from point to the centroid
  Double_t PC2 = matrixD2[0][0] + matrixD2[1][1] + matrixD2[2][2];
  fSumOfPC2 += PC2;

  fWeightSum += w;
  fNumPoints++;
}

void ODRFitter::SolveEigenValueEquation()
{
  (*fEigenVectors) = fMatrixA -> EigenVectors(*fEigenValues);
}

void ODRFitter::ChooseEigenValue(Int_t iEV)
{
  (*fNormal) = TMatrixDColumn((*fEigenVectors), iEV);
  fRMS = TMath::Sqrt(fSumOfPC2 - (*fEigenValues)[iEV]);
}

void ODRFitter::FitLine()
{
  SolveEigenValueEquation();
  ChooseEigenValue(0);
}

void ODRFitter::FitPlane()
{
  SolveEigenValueEquation();
  ChooseEigenValue(2);
}

TVector3 ODRFitter::GetCentroid()  { return TVector3((*fCentroid)[0][0], (*fCentroid)[1][0], (*fCentroid)[2][0]); }
TVector3 ODRFitter::GetNormal()    { return TVector3((*fNormal)[0], (*fNormal)[1], (*fNormal)[2]); }
TVector3 ODRFitter::GetDirection() { return TVector3((*fNormal)[0], (*fNormal)[1], (*fNormal)[2]); }
   Int_t ODRFitter::GetNumPoints() { return fNumPoints; }
Double_t ODRFitter::GetWeightSum() { return fWeightSum; }
Double_t ODRFitter::GetRMS()       { return fRMS; }
