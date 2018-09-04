/* Copyright 2008-2010, Technische Universitaet Muenchen,
   Authors: Christian Hoeppner & Sebastian Neubert & Johannes Rauch

   This file is part of GENFIT.

   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Rearranged by: Genie Jhang (geniejhang@nuclear.korea.ac.kr, Korea University)
*/

#include "STSpacepointMeasurement.hh"
#include "TMatrixD.h"
#include <cmath>
#include <iostream>
using namespace std;

ClassImp(genfit::STSpacepointMeasurement)

namespace genfit {

STSpacepointMeasurement::STSpacepointMeasurement()
:SpacepointMeasurement()
{}

STSpacepointMeasurement::STSpacepointMeasurement(const STHitCluster *cluster, const TrackCandHit *hit)
:SpacepointMeasurement()
{
  TVector3 pos = cluster -> GetPosition();
  TMatrixD mat = cluster -> GetCovMatrix();

  rawHitCoords_(0) = pos.X()/10.;
  rawHitCoords_(1) = pos.Y()/10.;
  rawHitCoords_(2) = pos.Z()/10.;

  TMatrixDSym cov(3);

  auto lambda = cluster -> GetLambda();
  auto chi = cluster -> GetChi();
  auto driftLength = -4.05 - cluster -> GetY();

  Double_t cx,cy,cz;
  Double_t parx0, parx1, parx2;
  Double_t pary0, pary1, pary2;
  Double_t parz0, parz1, parz2;
  Double_t cxmin, cymin, czmin;

  auto numHits = cluster -> GetNumHits();
  if (numHits == 1)
  {
    parx0 = 4;   parx1 = 0; parx2 = 0;
    pary0 = 0.8; pary1 = 0; pary2 = 0;
    parz0 = 1.8; parz1 = 0; parz2 = 0;

    cxmin = 2.46802;
    cymin = 0.699952;
    czmin = 1.55627;
  }
  else if (numHits == 2)
  {
    parx0 = -0.02; parx1 = 1.78; parx2 = 0.00087;
    pary0 = -0.01; pary1 = 0.93; pary2 = 0.00087;
    parz0 = 0.45; parz1 = 0.0431; parz2 = -0.00059;

    cxmin = 0.471073;
    cymin = 0.444433;
    czmin = 0.682794;
  }
  else if (numHits == 3)
  {
    parx0 = 0.25; parx1 = 0.87; parx2 = 0.00034;
    pary0 = 0.02; pary1 = 1.16; pary2 = 0.00078;
    parz0 = 0.55; parz1 = -0.0247; parz2 = -0.00017;

    cxmin = 0.563174;
    cymin = 0.443362;
    czmin = 0.634191;
  }
  else
  {
    parx0 = 2.34; parx1 = -3.31; parx2 = 0.00058;
    pary0 = 0.67; pary1 = 0.90; pary2 = 0.00054;
    parz0 = 2.92; parz1 = 0.2620; parz2 = -0.00279;

    cxmin = 1.2879;
    cymin = 0.808195;
    czmin = 1.63081;
  }

  cy = pary0 + pary1 * TMath::Tan(lambda)*TMath::Tan(lambda) + pary2 * driftLength;
  if (cluster -> IsLayerCluster()) {
    cx = parx0 + parx1 * TMath::Tan(chi)*TMath::Tan(chi) + parx2 * driftLength;
    cz = 12./TMath::Sqrt(12);
  }
  else {
    cx = 8./TMath::Sqrt(12);
    cz = parz0 + parz1 * TMath::Tan(chi)*TMath::Tan(chi) + parz2 * driftLength;
  }

  if (cx>3*cxmin) cx = 2*cxmin;
  if (cy>3*cymin) cy = 2*cymin;
  if (cz>3*czmin) cz = 2*czmin;

  if (cx<cxmin) cx = cxmin;
  if (cy<cymin) cy = cymin;
  if (cz<czmin) cz = czmin;

  cx = 0.01*cx; // to cm
  cy = 0.01*cy; // to cm
  cz = 0.01*cz; // to cm

  cov(0,0) = cx;
  cov(1,1) = cy;
  cov(2,2) = cz;

  rawHitCov_ = cov;
  detId_ = hit -> getDetId();
  hitId_ = hit -> getHitId();

  fCharge = cluster -> GetCharge();

  this -> initG();
}

AbsMeasurement *STSpacepointMeasurement::clone() const
{
  return new STSpacepointMeasurement(*this);
}

Double_t STSpacepointMeasurement::GetCharge() { return fCharge; }

} /* End of namespace genfit */
