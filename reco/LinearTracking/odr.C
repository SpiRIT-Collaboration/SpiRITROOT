TVector3 GetPointFromLineWithZ(TVector3 pointOnLine, TVector3 normal, Double_t z)
{
  Double_t zDiff = z - pointOnLine.Z();
  Double_t x = zDiff * normal.X()/normal.Z() + pointOnLine.X();
  Double_t y = zDiff * normal.Y()/normal.Z() + pointOnLine.Y();

  return TVector3(x,y,z);
}

void odr()
{
  TEveManager::Create();

  gStyle -> SetOptStat(0);

  const Int_t nPoints = 100;

  Double_t x[nPoints] = {0};
  Double_t y[nPoints] = {0};
  Double_t z[nPoints] = {0};
  Double_t w[nPoints] = {0};

  Double_t xMean = 0;
  Double_t yMean = 0;
  Double_t zMean = 0;

  Double_t factor = nPoints/10.;

  for (Int_t iPoint = 0; iPoint < nPoints; iPoint++)
  {
    x[iPoint] = gRandom -> Gaus(iPoint/factor, 0.1);
    y[iPoint] = gRandom -> Gaus(iPoint/factor, 0.1);
    z[iPoint] = gRandom -> Gaus(iPoint/factor, 0.1);
    w[iPoint] = gRandom -> Gaus(1, 0.5);

    xMean += w[iPoint]*x[iPoint];
    yMean += w[iPoint]*y[iPoint];
    zMean += w[iPoint]*z[iPoint];
  }

  xMean *= 1./nPoints;
  yMean *= 1./nPoints;
  zMean *= 1./nPoints;

  pointSet = new TEvePointSet("pointSet", nPoints);
  pointSet -> SetMarkerColor(kBlack);
  pointSet -> SetMarkerSize(2);
  pointSet -> SetMarkerColor(kBlack);
  pointSet -> SetMarkerStyle(20);

  for (Int_t iPoint = 0; iPoint < nPoints; iPoint++)
    pointSet -> SetNextPoint(x[iPoint], y[iPoint], z[iPoint]);

  gEve -> AddElement(pointSet);

  /*******************************************************************/

  ODRFitter *fitter = new ODRFitter();
  fitter -> SetCentroid(xMean, yMean, zMean);

  for (Int_t iPoint = 0; iPoint < nPoints; iPoint++)
    fitter -> AddPoint(x[iPoint], y[iPoint], z[iPoint], w[iPoint]);

  fitter -> FitLine();

  std::cout << "normal  : "; fitter -> GetDirection().Print();
  std::cout << "centroid: "; fitter -> GetCentroid().Print();

  TVector3 pointStart = GetPointFromLineWithZ(fitter -> GetCentroid(), fitter -> GetDirection(), 0);
  TVector3 pointEnd   = GetPointFromLineWithZ(fitter -> GetCentroid(), fitter -> GetDirection(), 10);

  std::cout << "start   : "; pointStart.Print();
  std::cout << "end     : "; pointEnd.Print();

  line = new TEveLine();
  line -> SetLineColor(kRed);

  line -> SetNextPoint(pointStart.X(), pointStart.Y(), pointStart.Z());
  line -> SetNextPoint(pointEnd.X(), pointEnd.Y(), pointEnd.Z());

  gEve -> AddElement(line);
  gEve -> GetDefaultGLViewer() -> SetClearColor(kWhite);
  gEve -> Redraw3D();
}
