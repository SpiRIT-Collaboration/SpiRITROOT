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

// Modified for SpiRITROOT by Genie Jhang (2015/06/15)

#ifndef STSPACEPOINTMEASUREMENT
#define STSPACEPOINTMEASUREMENT

#include "SpacepointMeasurement.h"
#include "TrackCandHit.h"

#include "STHitCluster.hh"

namespace genfit {

/** @brief Class for a spacepoint measurement which can be created
 *         from STHitCluster via the MeasurementFactory.
 *
 *  @author Johannes Rauch  (Technische Universit&auml;t M&uuml;nchen, original author)
 *  @author Genie Jhang (Korea University, modifier for S\piRITTPC)
 */
class STSpacepointMeasurement : public SpacepointMeasurement {
  public:
    STSpacepointMeasurement();
    STSpacepointMeasurement(const STHitCluster* detHit, const TrackCandHit* hit);

    virtual AbsMeasurement* clone() const;

    Double_t GetCharge();

  private:
    Double_t fCharge;

  ClassDef(STSpacepointMeasurement, 1)
};

} /* End of namespace genfit */

#endif
