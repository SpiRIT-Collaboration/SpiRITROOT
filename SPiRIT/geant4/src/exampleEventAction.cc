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
  eventID = 0;
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
    std::ofstream outFile("edep.out", std::ios::app);
    outFile << fKineticEnergy << " " 
            << eventID << " "
            << numHits << G4endl;

		for (G4int i = 0; i != numHits; i++)
		{
			G4double x = (*HC_Det)[i] -> GetX();
			G4double y = (*HC_Det)[i] -> GetY();
			G4double z = (*HC_Det)[i] -> GetZ();
			G4double e = (*HC_Det)[i] -> GetE();
      G4int parentID = (*HC_Det)[i] -> GetParentID();

      outFile << parentID << " "
              << x << " "
              << y << " "
              << z << " "
              << e << G4endl;
		}
	}

  eventID++;
}
