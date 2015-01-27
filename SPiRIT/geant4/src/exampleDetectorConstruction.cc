#include "exampleDetectorConstruction.hh"

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4UserLimits.hh"

#include "G4Box.hh"
#include "G4PVPlacement.hh"

#include "DetSD.hh"
#include "G4SDManager.hh"

#include "exampleDetectorROGeometry.hh"

#include "G4SystemOfUnits.hh"

exampleDetectorConstruction::exampleDetectorConstruction()
{
	ConstructMaterials();
	DefineDimensions();
}

exampleDetectorConstruction::~exampleDetectorConstruction()
{
	DestructMaterials();
}

void exampleDetectorConstruction::DefineDimensions()
{
	labX = 20*cm;
	labY = 20*cm;
	labZ = 200*cm;

	detX = 15*cm;
	detY = 15*cm;
	detZ = 144.64*cm;
}

G4VPhysicalVolume* exampleDetectorConstruction::Construct()
{
	G4Box* labSolid = new G4Box("labSolid", labX/2, labY/2, labZ/2);
	G4LogicalVolume* labLV = new G4LogicalVolume(labSolid, Air, "labLV");
	G4VPhysicalVolume* labPV = new G4PVPlacement(0, G4ThreeVector(), "labPV", labLV, 0, false, 0);

	G4Box* detSolid = new G4Box("detSolid", detX/2, detY/2, detZ/2);
	G4LogicalVolume *detLV = new G4LogicalVolume(detSolid, P10Gas, "detLV");
	new G4PVPlacement(0, G4ThreeVector(0, 0, 0), "detPV", detLV, labPV, false, 0);

	G4VisAttributes* detVisAttrib = new G4VisAttributes(G4Colour(0., 0., 1.));
	detLV -> SetVisAttributes(detVisAttrib);

  // Readout Geometry
	exampleDetectorROGeometry* detROGeometry = new exampleDetectorROGeometry("DetROGeometry",
																																					 labX, labY, labZ,
																																					 detX, detY, detZ,
																																					 G4ThreeVector(0, 0, 5*cm));

	detROGeometry -> BuildROGeometry();

	// Sensitive Detector
	G4SDManager* sdManager = G4SDManager::GetSDMpointer();

	DetSD* detSD = new DetSD("/det");
	detSD -> SetROgeometry(detROGeometry);
	sdManager -> AddNewDetector(detSD);
	detLV -> SetSensitiveDetector(detSD);

  //Set maximum step limit in TPC logical volume - JungWoo
  G4double maxStep = 1.*mm;
  G4UserLimits* stepLimit = new G4UserLimits(maxStep);
  detLV -> SetUserLimits(stepLimit);

	return labPV;
}

void exampleDetectorConstruction::ConstructMaterials()
{
	const G4double STP_Temperature = 273.15*kelvin;
	const G4double labTemp = STP_Temperature + 20.*kelvin;

	// Elements - G4Element(name, symbol, Z-number, molecular mass)
	elN = new G4Element("Nitrogen", "N", 7, 14.00674*g/mole);
	elO = new G4Element("Oxygen", "O", 8, 15.9994*g/mole);
	elAr = new G4Element("Argon", "Ar", 18, 39.948*g/mole);
	elC = new G4Element("Carbon", "C", 6, 12.011*g/mole);
	elH = new G4Element("Hydrogen", "H", 1, 1.00794*g/mole);

	// Materials - G4Material(name, density, # of element, state, temperature)
	Air = new G4Material("Air", 1.2929e-03*g/cm3, 3, kStateGas, labTemp);
		Air -> AddElement(elN, 75.47/99.95); 
		Air -> AddElement(elO, 23.20/99.95); 
		Air -> AddElement(elAr, 1.28/99.95); 

	Scint = new G4Material("Scintillator", 1.05*g/cm3, 2, kStateSolid, labTemp);
		Scint -> AddElement(elC, 10);
		Scint -> AddElement(elH, 11);

	Water = new G4Material("Water", 1*g/cm3, 2, kStateLiquid, labTemp);
		Water -> AddElement(elH, 2);
		Water -> AddElement(elO, 1);





  const G4double denArgonGas = 1.782e-03 *g/cm3 * STP_Temperature/labTemp;
  G4Material* ArgonGas = new G4Material("ArGas", 18, 39.948*g/mole, denArgonGas, kStateGas, labTemp);
    //ArgonGas -> AddElement(elAr, 1);

  const G4double denMethane = 0.717e-3 *g/cm3 * STP_Temperature/labTemp;
  G4Material* Methane = new G4Material("Methane", denMethane, 2, kStateGas, labTemp);
    Methane -> AddElement(elC, 1);
    Methane -> AddElement(elH, 4);
  
  const G4double denP10 = 1.53e-3*g/cm3;
  G4double tot = 0.9*denArgonGas + 0.1*denMethane;
  P10Gas = new G4Material("P10Gas", denP10, 2, kStateGas, labTemp);
    P10Gas -> AddMaterial(ArgonGas, 0.9*denArgonGas/tot);
    P10Gas -> AddMaterial(Methane,  0.1*denMethane/tot);

}

void exampleDetectorConstruction::DestructMaterials()
{
	delete Water;
	delete Scint;
	delete Air;

	delete elH;
	delete elC;
	delete elAr;
	delete elO;
	delete elN;
}
