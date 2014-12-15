#ifndef EXAMPLEDETECTORROGEOMETRY_h
#define EXAMPLEDETECTORROGEOMETRY_h 1

#include "G4VReadOutGeometry.hh"
#include "G4ThreeVector.hh"

class exampleDetectorROGeometry: public G4VReadOutGeometry
{
	public:
		exampleDetectorROGeometry(G4String aString,
											 G4double,
											 G4double,
											 G4double,
											 G4double,
											 G4double,
											 G4double,
											 G4ThreeVector);

		~exampleDetectorROGeometry();

	private:
		G4VPhysicalVolume* Build();

	private:
		G4double labX, labY, labZ;
		G4double detX, detY, detZ;
		G4ThreeVector detPosition;
};
#endif
