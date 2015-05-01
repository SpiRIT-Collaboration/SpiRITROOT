#include "G4VPhysicalVolume.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4TouchableHistory.hh"
#include "G4SDManager.hh"
#include "DetSD.hh"
#include "DetHit.hh"

DetSD::DetSD(const G4String &name)
:G4VSensitiveDetector(name)
{
	collectionName.insert("DetHitsCollection");
}

DetSD::~DetSD()
{
}

void DetSD::Initialize(G4HCofThisEvent *HCTE)
{
	hitsCollection = new G4THitsCollection<DetHit>(SensitiveDetectorName, collectionName[0]);

	G4int hcid = G4SDManager::GetSDMpointer() -> GetCollectionID(collectionName[0]);
	HCTE -> AddHitsCollection(hcid, hitsCollection);
}

G4bool DetSD::ProcessHits(G4Step *aStep, G4TouchableHistory *ROHist)
{
	if (!ROHist)	return false;

	G4double energyDeposit = aStep -> GetTotalEnergyDeposit();
	if (energyDeposit == 0.)	return false;

	if( (aStep -> GetPreStepPoint() -> GetPhysicalVolume() -> GetName() == "detPV") 
      && (energyDeposit != 0.)
      && (aStep -> GetTrack() -> GetParentID()==0)) 
  {
    G4double x = aStep -> GetPreStepPoint() -> GetPosition().getX();
    G4double y = aStep -> GetPreStepPoint() -> GetPosition().getY();
    G4double z = aStep -> GetPreStepPoint() -> GetPosition().getZ();
    G4double t = aStep -> GetPreStepPoint() -> GetGlobalTime();
    G4int parentID = aStep -> GetTrack() -> GetParentID();


		DetHit *aHit = new DetHit(x, y, z, t, energyDeposit, parentID);
		hitsCollection -> insert(aHit);
	}

	return true;
}

void DetSD::EndOfEvent(G4HCofThisEvent *HCTE)
{
}
