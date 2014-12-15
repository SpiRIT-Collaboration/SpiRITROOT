#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"

#include "exampleEventAction.hh"
#include "DetHit.hh"

#include <fstream>
#include <iomanip>

exampleEventAction::exampleEventAction(G4double kineticEnergy)
: fKineticEnergy(kineticEnergy)
{
}

exampleEventAction::~exampleEventAction()
{
}

void exampleEventAction::BeginOfEventAction(const G4Event* anEvent)
{
}

void exampleEventAction::EndOfEventAction(const G4Event* anEvent)
{
	G4SDManager* sdManager = G4SDManager::GetSDMpointer();

	G4HCofThisEvent* HCTE = anEvent -> GetHCofThisEvent();
	if (!HCTE) return;

	G4THitsCollection<DetHit>* HC_Det = NULL;

	hitsCollectionID = sdManager -> GetCollectionID("DetHitsCollection");
	HC_Det = (G4THitsCollection<DetHit> *)(HCTE -> GetHC(hitsCollectionID));

	if (HC_Det)
	{
		G4int numHits = HC_Det -> entries();

		for (G4int i = 0; i != numHits; i++)
		{
			G4double x = (*HC_Det)[i] -> GetX();
			G4double y = (*HC_Det)[i] -> GetY();
			G4double z = (*HC_Det)[i] -> GetZ();
			G4double t = (*HC_Det)[i] -> GetT();
			G4double e = (*HC_Det)[i] -> GetE();

			std::ofstream outFile("edep.out", std::ios::app);
			outFile << std::setw(15) << fKineticEnergy 
              << std::setw(15) << x 
              << std::setw(15) << y 
              << std::setw(15) << z 
              << std::setw(15) << t 
              << std::setw(15) << e << G4endl;
		}
	}
}
