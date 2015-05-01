#ifndef EXAMPLEEVENTACTION_h
#define EXAMPLEEVENTACTION_h 1

#include "G4UserEventAction.hh"

class G4Event;

class exampleEventAction: public G4UserEventAction
{
	public:
		exampleEventAction(G4double);
		virtual ~exampleEventAction();

		virtual void BeginOfEventAction(const G4Event* anEvent);
		virtual void EndOfEventAction(const G4Event* anEvent);

	private:
		G4int hitsCollectionID;

    G4double fKineticEnergy;

    G4int eventID;
};
#endif
