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

  const Int_t nPoints = 11;

  Double_t x[nPoints] = {0};
  Double_t y[nPoints] = {0};
  Double_t z[nPoints] = {0};
  Double_t w[nPoints] = {0};

  Double_t xMean = 0;
  Double_t yMean = 0;
  Double_t zMean = 0;

  Double_t factor = (nPoints-1)/10.;
  Double_t sigma = 0;

  for (Int_t iPoint = 0; iPoint < nPoints; iPoint++)
  {
    x[iPoint] = gRandom -> Gaus(iPoint/factor, sigma);
    y[iPoint] = gRandom -> Gaus(iPoint/factor, sigma);
    z[iPoint] = gRandom -> Gaus(iPoint/factor, sigma);
    w[iPoint] = gRandom -> Gaus(1, 0);

    cout << x[iPoint] << " " << y[iPoint] << " " << z[iPoint] << endl;

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

  ODRFitter *fODRFitter = new ODRFitter();
  fODRFitter -> SetCentroid(xMean, yMean, zMean);

  for (Int_t iPoint = 0; iPoint < nPoints; iPoint++)
    fODRFitter -> AddPoint(x[iPoint], y[iPoint], z[iPoint], w[iPoint]);

  fODRFitter -> FitLine();
  fODRFitter -> GetDirection().Print();
  fODRFitter -> GetCentroid().Print();

  STLinearTrackFitter* fLTFitter = new STLinearTrackFitter();

  STLinearTrack *track= new STLinearTrack();
  track -> SetDirection(fODRFitter -> GetDirection());
  track -> SetCentroid(fODRFitter -> GetCentroid());

  std::cout << fLTFitter -> PerpLine(track, TVector3(0,0,0)).Mag() << std::endl;
  TVector3 pointStart = fLTFitter -> GetClosestPointOnTrack(track, TVector3(0,0,0));
  TVector3 pointEnd   = fLTFitter -> GetClosestPointOnTrack(track, TVector3(10,10,10));
  //TVector3 pointStart = fLTFitter -> GetPointOnZ(track, -1);
  //TVector3 pointEnd   = fLTFitter -> GetPointOnZ(track, 11);

  pointStart.Print();
  pointEnd.Print();

  line = new TEveLine();
  line -> SetLineColor(kRed);

  line -> SetNextPoint(pointStart.X(), pointStart.Y(), pointStart.Z());
  line -> SetNextPoint(pointEnd.X(), pointEnd.Y(), pointEnd.Z());

  gEve -> AddElement(line);
  gEve -> GetDefaultGLViewer() -> SetClearColor(kWhite);
  gEve -> Redraw3D();
}
