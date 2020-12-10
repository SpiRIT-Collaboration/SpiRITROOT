#ifndef STFAIRMCEVENTHEADER_H
#define STFAIRMCEVENTHEADER_H

#include "FairMCEventHeader.h"
#include <map>

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
  int  TrackIDToZ(int id); // store Z value of each track. Will be used to identify the Z-value of a track when it hits Katana
  void RecordTrackZ(int id, int Z); 
  void ClearTrackZRecord();

  double GetBeamZ();
  double GetBeamA();
  double GetBeamAoZ();
  double GetEnergyPerNucleons();
  bool HasHvyResidue();
  void SetParentIDOnHits(bool val=true);
  bool IsParentIDOnHits();
protected:
  double fBeamZ;
  double fBeamA;
  double fBeamEnergyPerN;
  bool   fHasHvyResidue = false;
  std::map<int, int> fTrackID2Z;
  bool   fIsParentIDOnHits = false;

  ClassDef(STFairMCEventHeader, 1);
};

#endif
