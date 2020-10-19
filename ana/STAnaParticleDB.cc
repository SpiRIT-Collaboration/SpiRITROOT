#include "STAnaParticleDB.hh"
#include "TDatabasePDG.h"

#include <iostream>

ClassImp(STAnaParticleDB);

const double STAnaParticleDB::kAu2Gev = 0.9314943228;
const double STAnaParticleDB::khSlash = 1.0545726663e-27;
const double STAnaParticleDB::kErg2Gev = 1/1.6021773349e-3;
const double STAnaParticleDB::khShGev = khSlash*kErg2Gev;
const double STAnaParticleDB::kYear2Sec = 3600*24*365.25;
//extern std::vector<int> STAnaParticleDB::SupportedPDG;

void STAnaParticleDB::FillTDatabasePDG()
{
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

  if ( !pdgDB->GetParticle(1000020060) )
    pdgDB->AddParticle("He6","He6",6*kAu2Gev+2.424e-3,kTRUE,
                       0,6,"Ion",1000020060);


}

std::vector<int> STAnaParticleDB::EnableChargedParticles()
{
  std::vector<int> ChargedParticles{2212, 1000010020, 1000010030, 1000020030, 1000020040, 1000020060};
  SupportedPDG.insert(SupportedPDG.end(), ChargedParticles.begin(), ChargedParticles.end());
  return ChargedParticles; // return PDG added in this function
}

std::vector<int> STAnaParticleDB::EnablePions()
{
  std::vector<int> PionPDG{211, -211};
  SupportedPDG.insert(SupportedPDG.end(), PionPDG.begin(), PionPDG.end());
  return PionPDG; // return PDG added in this function
}

std::vector<int> STAnaParticleDB::EnableMCNeutrons()
{
  SupportedPDG.push_back(2112);
  return {2112};
}

std::vector<int> STAnaParticleDB::GetSupportedPDG()
{
  if(SupportedPDG.size() == 0)
  {
    std::cerr << "Please specify which particle type do you want to analyze with the following two commands,\n";
    std::cerr << "STAnaParticleDB::EnableChargedParticles()\n";
    std::cerr << "STAnaParticleDB::EnablePions() (if you want pions)\n";
    std::cerr << "Please be reminded that the order to which you enable things do matter.\n";
  }
  return SupportedPDG;
}

