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

ClassImp(genfit::STSpacepointMeasurement)

namespace genfit {

STSpacepointMeasurement::STSpacepointMeasurement()
:SpacepointMeasurement()
{}

STSpacepointMeasurement::STSpacepointMeasurement(const STHitCluster *detHit, const TrackCandHit *hit)
:SpacepointMeasurement()
{
  TVector3 pos = detHit -> GetPosition();

  rawHitCoords_(0) = pos.X()/10.;
  rawHitCoords_(1) = pos.Y()/10.;
  rawHitCoords_(2) = pos.Z()/10.;

  TVector3 posSigma = detHit -> GetPosSigma();
  TMatrixDSym cov(3);
  for (Int_t iComp = 0; iComp < 3; iComp++)
    cov(iComp, iComp) = posSigma(iComp)*posSigma(iComp)/100.;

  rawHitCov_ = cov;
  detId_ = hit -> getDetId();
  hitId_ = hit -> getHitId();

  this -> initG();
}

AbsMeasurement *STSpacepointMeasurement::clone() const
{
  return new STSpacepointMeasurement(*this);
}

} /* End of namespace genfit */
