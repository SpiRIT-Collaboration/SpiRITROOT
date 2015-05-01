#include "examplePrimaryGeneratorAction.hh"

#include "G4ParticleGun.hh"
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4UnitsTable.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleDefinition.hh"

#include "G4SystemOfUnits.hh"

examplePrimaryGeneratorAction::examplePrimaryGeneratorAction(G4double kineticEnergy)
{
	particleGun = new G4ParticleGun;

	gunPosition = G4ThreeVector(0, 0, -75*cm);
	particleGun -> SetParticlePosition(gunPosition);

	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	G4ParticleDefinition* proton = particleTable -> FindParticle("proton");
	//                          or particleTable -> GetIon(1, 1, 0);
	particleGun -> SetParticleDefinition(proton);

	momentumDirection = G4ThreeVector(0, 0, 1);
	particleGun -> SetParticleMomentumDirection(momentumDirection);

	kineticEnergy = kineticEnergy*MeV;
	particleGun -> SetParticleEnergy(kineticEnergy);
}

examplePrimaryGeneratorAction::~examplePrimaryGeneratorAction()
{
	delete particleGun;
}

void examplePrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	particleGun -> GeneratePrimaryVertex(anEvent);
}
