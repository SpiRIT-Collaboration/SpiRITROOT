#include "STAnaParticleDB.hh"
#include "TDatabasePDG.h"

const double STAnaParticleDB::kAu2Gev = 0.9314943228;
const double STAnaParticleDB::khSlash = 1.0545726663e-27;
const double STAnaParticleDB::kErg2Gev = 1/1.6021773349e-3;
const double STAnaParticleDB::khShGev = khSlash*kErg2Gev;
const double STAnaParticleDB::kYear2Sec = 3600*24*365.25;

void STAnaParticleDB::FillTDatabasePDG()
{
  using namespace STAnaParticleDB;
  auto pdgDB = TDatabasePDG::Instance();
  if ( !pdgDB->GetParticle(1000010020) )
    pdgDB->AddParticle("Deuteron","Deuteron",2*kAu2Gev+8.071e-3,kTRUE,
                       0,3,"Ion",1000010020);

  if ( !pdgDB->GetParticle(1000010030) )
    pdgDB->AddParticle("Triton","Triton",3*kAu2Gev+14.931e-3,kFALSE,
                       khShGev/(12.33*kYear2Sec),3,"Ion",1000010030);

  if ( !pdgDB->GetParticle(1000020040) )
    pdgDB->AddParticle("He4","He4",4*kAu2Gev+2.424e-3,kTRUE,
                       khShGev/(12.33*kYear2Sec),6,"Ion",1000020040);

  if ( !pdgDB->GetParticle(1000020030) )
    pdgDB->AddParticle("He3","He3",3*kAu2Gev+14.931e-3,kFALSE,
                       0,6,"Ion",1000020030); 
}
