#include "TMath.h"

#include "STBDCProjection.hh"

#include <fstream>
#include <iostream>
using namespace std;

STBDCProjection::STBDCProjection(TString reducedBMap)
{
  cout << "Loding " << reducedBMap.Data() << endl;
//  TLoadField();
  Int_t ii = 0;
  Double_t tx, ty, tz;
  Double_t tbx, tby, tbz;

  std::ifstream Bfield;
  Bfield.open(reducedBMap.Data());
  while (ii <= 300) {
    Bfield >> tx >> ty >> tz >> tbx >> tby >> tbz;
    m_Bxx[ii] = tbx;
    m_Byy[ii] = tby;
    m_Bzz[ii] = tbz;

    ii++;

    if (!Bfield.good())
      break;
  }
  Bfield.close();
}

/*
STBDCProjection::STBDCProjection(Double_t bdc1z, Double_t bdc2z, Double_t TGTz)
{
  m_bdc1z = bdc1z;
  m_bdc2z = bdc2z;
  m_end_z = TGTz;

  TLoadField();
}
*/

/*
void STBDCProjection::TLoadField(){
  Int_t ii = 0;
  Double_t tx, ty, tz;
  Double_t tbx, tby, tbz;

  std::ifstream Bfield;
  Bfield.open("/input/ReducedBMap.txt");
  while (ii <= 300) {
    Bfield >> tx >> ty >> tz >> tbx >> tby >> tbz;
    m_Bxx[ii] = tbx;
    m_Byy[ii] = tby;
    m_Bzz[ii] = tbz;

    ii++;

    if (!Bfield.good())
      break;
  }
  Bfield.close();
}
*/

void STBDCProjection::ProjectParticle(Double_t sx, Double_t sy, Double_t sz, Double_t sa, Double_t sb, Double_t charge, Double_t energy, Double_t endz, Double_t mass){
  m_mass = mass;
  m_energy = energy;
  //m_mass=131.8904*931.494;//calibration check
  //m_energy=m_mass+37345;//calibration check

  setKE();

  Double_t Elost = 0;
  Double_t E132[9] = {0.998375283,0.9962917511,0.9998661133,0.9921531869,0.9554079952,0.9522248905,0.9997032993,0.9995844958,0.9509204276};//percent of KE left after total material, Sn132
  Double_t E124[9] = {0.9982873828,0.9960907838,0.9998870631,0.9917546733,0.9521382609,0.9484465177,0.9997162405,0.9995269333,0.9478749251};//percent of KE left after total material, Sn124
  Double_t E112[9] = {0.9981455152,0.9957267604,0.999844508,0.9909489596,0.9486189579,0.9441154088,0.9996495409,0.9995091852,0.9418098913};//percent of KE left after total material, Sn112
  Double_t E108[9] = {0.9980763682,0.9955349973,0.9998709344,0.9905769975,0.9452371645,0.9403412028,0.9996334848,0.9994866906,0.9388114453};//percent of KE left after total material, Sn108

  for (Int_t i = 0; i < 9; i++) {
    if (m_beam > 130 && m_beam < 135) Eloss[i] = (m_kinetic_energy - Elost)*(1 - E132[i]);
    else if (m_beam > 120 && m_beam < 130) Eloss[i] = (m_kinetic_energy - Elost)*(1 - E124[i]);
    else if (m_beam > 110 && m_beam < 120) Eloss[i] = (m_kinetic_energy - Elost)*(1 - E112[i]);
    else if (m_beam > 100 && m_beam < 110) Eloss[i] = (m_kinetic_energy - Elost)*(1 - E108[i]);

    Elost = Elost + Eloss[i];
  }

  //setMomentum();
  setBeta();
  setMomentum();

  m_x = sx;
  m_y = sy;
  m_z = sz;
  m_a = sa;
  m_b = sb;
  m_end_z = endz;
  m_charge = charge;
  m_pz=m_momentum/std::sqrt(1+std::tan(m_a/1000.)*std::tan(m_a/1000.)+std::tan(m_b/1000.)*std::tan(m_b/1000.));
  m_px=m_pz*std::tan(m_a/1000.);
  m_py=m_pz*std::tan(m_b/1000.);
  m_brho=3.3356*std::sqrt(m_momentum*m_momentum-m_py*m_py)/m_charge/1000.; //given charge in multiples of proton charge, momentum in MeV/c, provides Brho in T*m
  //TLoadField();
  m_By = 0;

  if (m_brho > 0) {
    while (m_z < m_end_z && TMath::Abs(m_x) < 430.) {
      m_By = getByField(m_x, m_y, m_z);
      dz = 1.;  //step size in mm

      if (m_z + dz > m_end_z)
        dz = m_end_z - m_z;

      MagStep();
    }
  }

  m_pz=m_momentum/std::sqrt(1+std::tan(m_a/1000.)*std::tan(m_a/1000.)+std::tan(m_b/1000.)*std::tan(m_b/1000.));
  m_px=m_pz*std::tan(m_a/1000.);
  m_py=m_pz*std::tan(m_b/1000.);

}
void STBDCProjection::MagStep() {
  Double_t da = 0.;  //no change in angle unless |B|>0
  m_brho = 3.3356*std::sqrt(m_momentum*m_momentum-m_py*m_py)/m_charge/1000.;
  dx = -dz*TMath::Tan(m_a/1000.);  //linear projection used unless |B|>0
  dy = dz*TMath::Tan(m_b/1000.);
  if (TMath::Abs(m_By) > 0.) {
    Double_t mrho = m_brho/m_By*1000.;
    da = -TMath::ASin(dz/mrho)*1000.;  //gives da in mrad
    dx = mrho*(TMath::Cos((m_a + da)/1000.) - TMath::Cos(m_a/1000.));  //gives dx in mm
  }
  dr = TMath::Sqrt(dz*dz + dy*dy + dx*dx);
  m_x = m_x + dx;
  m_y = m_y + dy;
  m_a = m_a + da;
  m_z = m_z + dz;

  updateEnergy2();
  setMomentum();
  setBeta();

}
void STBDCProjection::updateEnergy2() {  //use a table of energy loss through material (from LISE++) to apply energy loss
  Double_t thickness[9] = {0.05,141.75,0.004,274.184,0.418,0.418,8.316,0.012,1447.};                    //thickness of material
  Double_t bound1[9] = {-1009.05,-1009.,-867.25,-867.246,-593.062,-592.644,-592.226,-583.91,-583.898};  //start of each material
  Double_t bound2[9] = {-1009.,-867.25,-867.246,-593.062,-592.644,-592.226,-583.91,-583.898,863.102};   //end of each material

  for(Int_t i = 0; i < 9; i++) {  //for each material, determine how much of the material the step includes, and apply appropriate correction
    // if(m_z>bound2[i]) break;
    Double_t eff_thick = 0.;
    eff_thick = TMath::Min(bound2[i], m_z) - TMath::Max(bound1[i], m_z - dz);
    if (eff_thick > 0) {
      if ((eff_thick/thickness[i]*Eloss[i]) > 0)
        m_kinetic_energy = m_kinetic_energy - (eff_thick*Eloss[i]/thickness[i]*dr/dz);

      m_energy = m_kinetic_energy + m_mass;

      setBeta();
      setMomentum();
    }

  }
}
void STBDCProjection::setMomentum(){
  m_momentum = TMath::Min(TMath::Sqrt(m_energy*m_energy - m_mass*m_mass), m_mass*m_beta/TMath::Sqrt(TMath::Abs(1 - m_beta*m_beta))); //I assume that momentum is always lost
  //m_momentum=m_mass*m_beta/TMath::Sqrt(TMath::Abs(1-m_beta*m_beta));
}

void STBDCProjection::setBeta() {
  m_beta = TMath::Sqrt(1 - m_mass*m_mass/m_energy/m_energy);
}

Double_t STBDCProjection::getByField(Double_t x, Double_t y, Double_t z){
  m_By = 0.;

  if (TMath::Sqrt(x*x + z*z) < 3000) {
    Int_t t_index = (Int_t)(TMath::Sqrt(z*z + x*x)/10. + 0.5);
    m_By = m_Byy[t_index];
  }

  return m_By;
}

Double_t STBDCProjection::getX()    { return m_x; }
Double_t STBDCProjection::getY()    { return m_y; }
Double_t STBDCProjection::getZ()    { return m_z; }
Double_t STBDCProjection::getA()    { return m_a; }
Double_t STBDCProjection::getB()    { return m_b; }
Double_t STBDCProjection::getPX()   { return m_px; }
Double_t STBDCProjection::getPY()   { return m_py; }
Double_t STBDCProjection::getPZ()   { return m_pz; }
    void STBDCProjection::setE()    { m_energy = m_mass/TMath::Sqrt(1 - m_beta*m_beta); }
Double_t STBDCProjection::getE()    { return m_energy; }
    void STBDCProjection::setKE()   { m_kinetic_energy = m_energy - m_mass; }
Double_t STBDCProjection::getKE()   { return m_kinetic_energy; }
Double_t STBDCProjection::getMeVu() { return m_kinetic_energy/(m_mass/931.494); }
Double_t STBDCProjection::getP()    { return m_momentum; }
Double_t STBDCProjection::getBeta() { return m_beta; }

void STBDCProjection::setBeam(Int_t runNo)
{
  if (runNo>=2174 && runNo<=2509) m_beam=108;
  if (runNo>=2520 && runNo<=2653) m_beam=112;
  if (runNo>=3044 && runNo<=3184) m_beam=124;
  if (runNo>=2819 && runNo<=3039) m_beam=132;
}

Int_t STBDCProjection::getBeam() { return m_beam; }
