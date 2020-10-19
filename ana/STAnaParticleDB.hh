#ifndef STANAPARTICLEDB_
#define STANAPARTICLEDB_

#include <vector>
#include "TObject.h"

// this should have been a namespace
// but ROOT interpreter doesn't autoload static namespace function until a class from ana is loaded
// work around by implementing namespace as class to force autoload
struct STAnaParticleDB : public TObject
{
  static const double kAu2Gev;// = 0.9314943228;
  static const double khSlash;// = 1.0545726663e-27;
  static const double kErg2Gev;// = 1/1.6021773349e-3;
  static const double khShGev;// = khSlash*kErg2Gev;
  static const double kYear2Sec;// = 3600*24*365.25;
  static std::vector<int> SupportedPDG;//, 211, -211};
  static void FillTDatabasePDG();
  static std::vector<int> EnableChargedParticles();
  static std::vector<int> EnablePions();
  static std::vector<int> EnableMCNeutrons();
  static std::vector<int> GetSupportedPDG();

  ClassDef(STAnaParticleDB, 1);
};

#endif
