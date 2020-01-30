#ifndef STANAPARTICLEDB_
#define STANAPARTICLEDB_

namespace STAnaParticleDB
{
  extern const double kAu2Gev;// = 0.9314943228;
  extern const double khSlash;// = 1.0545726663e-27;
  extern const double kErg2Gev;// = 1/1.6021773349e-3;
  extern const double khShGev;// = khSlash*kErg2Gev;
  extern const double kYear2Sec;// = 3600*24*365.25;
  void FillTDatabasePDG();
};

#endif
