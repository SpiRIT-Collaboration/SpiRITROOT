#ifndef URQMDPARTICLE_H
#define URQMDPARTICLE_H 1

#include "TObject.h"
#include "TLorentzVector.h"

class UrQMDParticle : public TObject
{
  public:
    UrQMDParticle();
    UrQMDParticle(Int_t,Int_t,Int_t,Int_t,TLorentzVector,TLorentzVector);
    virtual ~UrQMDParticle();
    UrQMDParticle(const UrQMDParticle &right);
    const UrQMDParticle& operator=(const UrQMDParticle &right);

  private:
    Int_t fPdg;                // pdg code
    Int_t fCharge;             // charge
    Int_t fZ;                  // # of proton (for meson, fZ=0)
    Int_t fN;                  // # of neutron (for meson, fN=0)
    TLorentzVector fMomentum;  // 4-d momentum for the particle (PX,PY,PZ,E) [MeV/c,MeV/c,MeV/c,MeV]
    TLorentzVector fPosition;  // 4-d position (X,Y,Z,T) [fm,fm,fm,fm/c]


  public:
    void SetPdg(Int_t pdg)  { fPdg = pdg; }
    void SetCharge(Int_t c) { fCharge = c; }
    void SetZ(Int_t z)      { fZ = z; }
    void SetN(Int_t n)      { fN = n; }
    void SetMomentum(TLorentzVector mom)   { fMomentum = mom; }
    void SetPosition(TLorentzVector pos)   { fPosition = pos; }

    Int_t GetPdg() { return fPdg; }
    Int_t GetCharge() { return fCharge; }
    Int_t GetZ()   { return fZ; }
    Int_t GetN()   { return fN; }
    Int_t GetA()   { return fZ+fN; }
    TLorentzVector GetMomentum()  { return fMomentum; }
    TLorentzVector GetPosition()  { return fPosition; }

    ClassDef(UrQMDParticle,1);

};

#endif
