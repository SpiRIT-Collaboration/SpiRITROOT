#include "STFairMCEventHeader.hh"
#include "FairRootManager.h"

STFairMCEventHeader::STFairMCEventHeader()
{}

STFairMCEventHeader::~STFairMCEventHeader()
{}

void STFairMCEventHeader::SetHvyResidue(bool value)
{ fHasHvyResidue = value; }

void STFairMCEventHeader::SetBeamZ(double Z)
{ fBeamZ = Z; }

void STFairMCEventHeader::SetBeamA(double A)
{ fBeamA = A; }

void STFairMCEventHeader::SetEnergyPerNucleons(double EnergyPN)
{ fBeamEnergyPerN = EnergyPN; }

double STFairMCEventHeader::GetBeamZ()
{ return fBeamZ; }

double STFairMCEventHeader::GetBeamA()
{ return fBeamA; }

double STFairMCEventHeader::GetBeamAoZ()
{ return double(fBeamA)/fBeamZ; }

double STFairMCEventHeader::GetEnergyPerNucleons()
{ return fBeamEnergyPerN; }

bool STFairMCEventHeader::HasHvyResidue()
{ return fHasHvyResidue; }

void STFairMCEventHeader::Register()
{ 
  FairRootManager::Instance()->Register("MCEventHeader.", "Event", this, kTRUE);
}
