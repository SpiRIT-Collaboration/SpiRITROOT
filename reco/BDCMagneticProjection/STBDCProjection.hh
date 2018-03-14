#ifndef STBDCPROJECTION_HH
#define STBDCPROJECTION_HH

#include "TString.h"

class STBDCProjection {
  public:
    STBDCProjection(TString reducedBMap);
//    STBDCProjection(Double_t bdc1z, Double_t bdc2z, Double_t TGTz);

    virtual ~STBDCProjection() {};

    void ProjectParticle(Double_t sx, Double_t sy, Double_t sz, Double_t sa, Double_t sb, Double_t charge, Double_t energy, Double_t endz, Double_t mass);

    Double_t getX();
    Double_t getY();
    Double_t getZ();
    Double_t getA();
    Double_t getB();
    Double_t getPX();
    Double_t getPY();
    Double_t getPZ();
    Double_t getE();
    Double_t getKE();
    Double_t getMeVu();
    Double_t getP();
    Double_t getBeta();

    void setBeam(Int_t runNo);
    Int_t getBeam();

  private:
    Int_t m_beam;
    Double_t m_beta;
    Double_t m_brho;
    Double_t m_charge;
    Double_t m_aoq;
    Double_t m_mass;
    Double_t m_energy;
    Double_t m_kinetic_energy;
    Double_t m_momentum;
    Double_t m_bdc1x;
    Double_t m_bdc1y;
    Double_t m_bdc1z;
    Double_t m_bdc2x;
    Double_t m_bdc2y;
    Double_t m_bdc2z;
    Double_t m_x;
    Double_t m_y;
    Double_t m_z;
    Double_t m_px;
    Double_t m_py;
    Double_t m_pz;
    Double_t m_a;
    Double_t m_b;
    Double_t m_Bxx[300];
    Double_t m_Byy[300];
    Double_t m_Bzz[300];
    Double_t Eloss[9];
    Double_t m_Bx;
    Double_t m_By;
    Double_t m_Bz;
    Double_t m_end_z;
    Double_t dx;
    Double_t dy;
    Double_t dz;
    Double_t dr;

    void MagStep();
//    void TLoadField();
    void updateEnergy2();
    void setMomentum();
    void setBeta();
    void setE();
    void setKE();

    Double_t getByField(Double_t x, Double_t y, Double_t z);
};
#endif
