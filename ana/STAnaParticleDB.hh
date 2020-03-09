#ifndef STANAPARTICLEDB_
#define STANAPARTICLEDB_

#include <vector>

namespace STAnaParticleDB
{
  extern const double kAu2Gev;// = 0.9314943228;
  extern const double khSlash;// = 1.0545726663e-27;
  extern const double kErg2Gev;// = 1/1.6021773349e-3;
  extern const double khShGev;// = khSlash*kErg2Gev;
  extern const double kYear2Sec;// = 3600*24*365.25;
  static const std::vector<int> SupportedPDG{2212, 1000010020, 1000010030, 1000020030, 1000020040, 1000020060};
  void FillTDatabasePDG();
};

#endif
