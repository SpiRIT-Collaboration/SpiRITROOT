#include "STFairMCEventHeader.hh"
#include "FairRootManager.h"

STFairMCEventHeader::STFairMCEventHeader()
{}

STFairMCEventHeader::~STFairMCEventHeader()
{}

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

void STFairMCEventHeader::Register()
{ 
  FairRootManager::Instance()->Register("MCEventHeader.", "Event", this, kTRUE);
}
