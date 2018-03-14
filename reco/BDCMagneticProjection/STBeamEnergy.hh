#ifndef STBEAMENERGY_HH
#define STBEAMENERGY_HH

class STBeamEnergy {
  public:
    STBeamEnergy();
    STBeamEnergy(Double_t z, Double_t aoq, Double_t beam78);

    virtual ~STBeamEnergy() {};

    void reset(Double_t z, Double_t aoq, Double_t beam78);

    void setBeta78(Double_t beta78);
    void setZ(Double_t z);
    void setAoq(Double_t aoq);
    void setCorrection();

    Double_t getBeta78();
    Double_t getZ();
    Double_t getAoq();
    Double_t setMass();
    Double_t getMass();  //returns estimated particle mass

    void setBeam(Int_t runNo);
    Int_t getBeam();

    Double_t setEnergy78();
    Double_t getEnergy78();  //returns initial energy
    Double_t getBeta();
    Double_t getBrho();
    Double_t getCorrectedEnergy();  //returns energy after BDC2
    Double_t getCorrectedMomentum();

  private:
    Int_t m_beam;
    Double_t m_beta;
    Double_t m_brho;
    Double_t m_beta78;
    Double_t m_z;
    Double_t m_aoq;
    Double_t m_mass;
    Double_t m_energy78;
    Double_t m_energy;
    Double_t m_momentum;
    bool m_corr;
};

#endif
