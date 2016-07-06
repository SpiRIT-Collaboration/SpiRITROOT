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

/**
 * @brief STVertex Class
 *
 * @author Genie Jhang
 *
 * @info
 * The same class as GFRaveVertex but the unit of the position and covariance matrix is in mm.
 */

#include "STVertex.hh"
#include <Exception.h>

#include <iostream>

STVertex::STVertex()
:fCov(3, 3), fNDF(0), fChi2(0), fID(-1)
{
}

STVertex::STVertex(const genfit::GFRaveVertex &vertex):
  fNDF(vertex.getNdf()),
  fChi2(vertex.getChi2()),
  fID(vertex.getId())
{
  fPos = vertex.getPos()*10.;
  fCov.ResizeTo(vertex.getCov());
  fCov = vertex.getCov()*100.;

  UInt_t nPar = vertex.getNTracks();
  fSmoothedTracks.reserve(nPar);
  for (UInt_t i = 0; i < nPar; i++) {
    fSmoothedTracks.push_back(new genfit::GFRaveTrackParameters(*(vertex.getParameters(i))));
  }
}

STVertex::~STVertex() {
  UInt_t nPar = fSmoothedTracks.size();
  for (UInt_t i = 0; i < nPar; i++)
    delete fSmoothedTracks[i];
}

void STVertex::Print(const Option_t *) const {
  std::cout << "STVertex\n";
  std::cout << "Position: "; GetPos().Print();
  std::cout << "Covariance: "; GetCov().Print();
  std::cout << "Ndf: " << GetNDF() << ", Chi2: " << GetChi2() << ", ID: " << GetID() << "\n";
  std::cout << "Number of tracks: " << GetNTracks() << "\n";
  for (UInt_t i = 0; i < GetNTracks(); i++) {
    std::cout << " track " << i << ":\n"; GetParameters(i) -> Print();
  }
}
