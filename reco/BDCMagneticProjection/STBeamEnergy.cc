#include "TMath.h"

#include "STBeamEnergy.hh"

STBeamEnergy::STBeamEnergy() { m_corr = kFALSE; }
STBeamEnergy::STBeamEnergy(Double_t z, Double_t aoq, Double_t beta78) {
  reset(z, aoq, beta78);
}

void STBeamEnergy::reset(Double_t z, Double_t aoq, Double_t beta78) {
  m_corr = kFALSE;
  setBeta78(beta78);
  setZ(z);
  setAoq(aoq);
  setMass();
  setEnergy78();
}

void STBeamEnergy::setBeta78(Double_t beta78) {
  m_beta78 = beta78;
}

Double_t STBeamEnergy::getBeta78() {
  return m_beta78;
}

void STBeamEnergy::setCorrection() {  //use calibrate energy loss
  Double_t dE132 = 0.9374555242;
  Double_t dE124 = 0.9384493375;
  Double_t dE112 = 0.9328597049;
  Double_t dE108 = 0.9308606678;
  //setAoq(2.64);
  //m_mass = 160440.1;
  //setEnergy78();
  //Double_t kinetic_energy = 39903;//m_energy78 - m_mass;
  Double_t kinetic_energy = m_energy78 - m_mass;

       if (m_beam > 130 && m_beam < 140) kinetic_energy = kinetic_energy*dE132;
  else if (m_beam > 120 && m_beam < 130) kinetic_energy = kinetic_energy*dE124;
  else if (m_beam > 110 && m_beam < 120) kinetic_energy = kinetic_energy*dE112;
  else if (m_beam > 100 && m_beam < 110) kinetic_energy = kinetic_energy*dE108;

  m_energy = kinetic_energy + m_mass;
  m_beta = TMath::Sqrt(1 - (m_mass/(m_energy))*(m_mass/(m_energy)));
  //m_momentum = m_mass*m_beta/TMath::Sqrt(std::abs(1 - m_beta*m_beta));
  m_momentum = TMath::Sqrt(m_energy*m_energy - m_mass*m_mass);
  //m_energy = m_mass*(1/TMath::Sqrt(1 - m_beta*m_beta));
  m_corr = kTRUE;
  //std::cout << kinetic_energy << std::endl;
}

Double_t STBeamEnergy::getBeta() {
  if (!m_corr)
    setCorrection();

  return m_beta;
}
Double_t STBeamEnergy::getBrho() {
  if (!m_corr)
    setCorrection();

  m_brho = 3.3356*m_momentum/m_z/1000.;
  return m_brho;
}
void STBeamEnergy::setZ(Double_t z) {
  m_z = z;
}
Double_t STBeamEnergy::getZ() {
  return m_z;
}
void STBeamEnergy::setAoq(Double_t aoq) {
  m_aoq = aoq;
}
Double_t STBeamEnergy::getAoq() {
  return m_aoq;
}
Double_t STBeamEnergy::setMass() {
  return m_mass = /*(Double_t)((Int_t)*/((m_aoq*m_z)*931.494*0.9993774);
}
Double_t STBeamEnergy::getMass() {
  return m_mass;
}
Double_t STBeamEnergy::setEnergy78() {
  return m_energy78 = m_mass*(1/TMath::Sqrt(1 - m_beta78*m_beta78));
}

Double_t STBeamEnergy::getEnergy78() {
  return m_energy78;
}

Double_t STBeamEnergy::getCorrectedEnergy() {
  if (!m_corr)
    setCorrection();

  return m_energy;
}

Double_t STBeamEnergy::getCorrectedMomentum() {
  if (!m_corr)
    setCorrection();

  return m_momentum;
}

void STBeamEnergy::setBeam(Int_t runNo) {
       if (runNo >= 2174 && runNo <= 2509) m_beam = 108;
  else if (runNo >= 2520 && runNo <= 2653) m_beam = 112;
  else if (runNo >= 3044 && runNo <= 3184) m_beam = 124;
  else if (runNo >= 2819 && runNo <= 3039) m_beam = 132;
}

Int_t STBeamEnergy::getBeam() {
  return m_beam;
}
