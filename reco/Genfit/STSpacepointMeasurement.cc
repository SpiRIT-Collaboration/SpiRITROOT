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

  rawHitCoords_(0) = pos.X()/10.;
  rawHitCoords_(1) = pos.Y()/10.;
  rawHitCoords_(2) = pos.Z()/10.;

  TMatrixDSym cov(3);

  if (cluster -> GetCharge() < -49)
  {
    cov(0,0) = 0.000001;
    cov(1,1) = 0.000001;
    cov(2,2) = 0.000001;
  }
  else 
  {
    auto lambda = cluster -> GetLambda();
    auto chi = cluster -> GetChi();

    Double_t cx, cy, cz;

    cy = 0.3433 + 0.006847 * lambda + 0.747 * lambda * lambda;
    if (cluster -> IsLayerCluster()) {
      cx = 0.3847 + 0.5708 * chi + 0.4104 * chi * chi;
      cz = TMath::Sqrt(12);
    } else {
      cx = 8./TMath::Sqrt(12);
      cz = 2.745 + -3644 * chi + 1.593 * chi * chi;
    }

    if (cx>8./TMath::Sqrt(12)) cx = 8./TMath::Sqrt(12);
    if (cy>2.) cy = 2.;
    if (cz>TMath::Sqrt(12)) cz = TMath::Sqrt(12);

    if (cx<0.4) cx = 0.4;
    if (cy<0.4) cy = 0.4;
    if (cz<0.4) cz = 0.4;

    cx = 0.01*cx;
    cy = 0.01*cy;
    cz = 0.01*cz;

    cov(0,0) = cx;
    cov(1,1) = cy;
    cov(2,2) = cz;
  }

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
