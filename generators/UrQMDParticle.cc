#include "UrQMDParticle.hh"

ClassImp(UrQMDParticle);

UrQMDParticle::UrQMDParticle()
  : TObject(), fPdg(), fCharge(), fZ(), fN(),
  fMomentum(TLorentzVector()), fPosition(TLorentzVector())
{}

UrQMDParticle::UrQMDParticle(Int_t pdg, Int_t c, Int_t z, Int_t n, TLorentzVector mom, TLorentzVector pos)
  : TObject(), fPdg(pdg), fCharge(c), fZ(z), fN(n),
  fMomentum(TLorentzVector(mom.Vect(),mom.E())), fPosition(TLorentzVector(pos.Vect(),pos.T()))
{}


UrQMDParticle::UrQMDParticle(const UrQMDParticle &right)
  : TObject()
{
  fPdg = right.fPdg;
  fCharge = right.fCharge;
  fZ = right.fZ;
  fN = right.fN;
  fMomentum = right.fMomentum;
  fPosition = right.fPosition;
}

const UrQMDParticle& UrQMDParticle::operator=(const UrQMDParticle &right)
{
  fPdg = right.fPdg;
  fCharge = right.fCharge;
  fZ = right.fZ;
  fN = right.fN;
  fMomentum = right.fMomentum;
  fPosition = right.fPosition;
  return *this;
}

UrQMDParticle::~UrQMDParticle()
{}
