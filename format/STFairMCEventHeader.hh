#ifndef STFAIRMCEVENTHEADER_H
#define STFAIRMCEVENTHEADER_H

#include "FairMCEventHeader.h"

class STFairMCEventHeader : public FairMCEventHeader
{
public: 
  // default constructor
  STFairMCEventHeader();

  virtual ~STFairMCEventHeader();
  virtual void Register();

  void SetHvyResidue(bool value=true);
  void SetBeamZ(double Z);
  void SetBeamA(double A);
  void SetEnergyPerNucleons(double EnergyPN); // in GeV

  double GetBeamZ();
  double GetBeamA();
  double GetBeamAoZ();
  double GetEnergyPerNucleons();
  bool HasHvyResidue();
protected:
  double fBeamZ;
  double fBeamA;
  double fBeamEnergyPerN;
  bool   fHasHvyResidue = false;

  ClassDef(STFairMCEventHeader, 1);
};

#endif
