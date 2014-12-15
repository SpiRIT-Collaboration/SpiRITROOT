#ifndef exampleDetectorConstruction_h
#define exampleDetectorConstruction_h 1

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"

#include "G4Element.hh"
#include "G4Material.hh"

class G4VPhysicalVolume;

class exampleDetectorConstruction: public G4VUserDetectorConstruction
{
	public:
		exampleDetectorConstruction();
		virtual ~exampleDetectorConstruction();

		virtual G4VPhysicalVolume* Construct();

	private:
		// comment: When you change dimensions of a detector in the interactive
		// 					mode, dimensions should be set out of Construct().
		void DefineDimensions();

		// comment: You'd better define all material pointers here and initialize
		//					them as 0 in the constructor. Finally, allocate them in
		//					function below.
		void ConstructMaterials();
		void DestructMaterials();

	private:
		G4Element* elN;
		G4Element* elO;
		G4Element* elAr;
		G4Element* elC;
		G4Element* elH;

		G4Material* Air;
		G4Material* Scint;
		G4Material* Water;

		G4Material* P10Gas;

		G4double labX, labY, labZ;
		G4double detX, detY, detZ;
};
#endif
