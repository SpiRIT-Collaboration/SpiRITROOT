#include "exampleDetectorROGeometry.hh"

#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4Material.hh"

#include "DetDummySD.hh"

#include "G4SystemOfUnits.hh"

exampleDetectorROGeometry::exampleDetectorROGeometry(G4String aString,
																						  G4double val1,
																							G4double val2,
																							G4double val3,
																							G4double val4,
																							G4double val5,
																							G4double val6,
																							G4ThreeVector val7)
:G4VReadOutGeometry(aString),
 labX(val1), labY(val2), labZ(val3),
 detX(val4), detY(val5), detZ(val6),
 detPosition(val7)
{
}

exampleDetectorROGeometry::~exampleDetectorROGeometry()
{
}

G4VPhysicalVolume* exampleDetectorROGeometry::Build()
{
	G4Material* dummyMat = new G4Material("dummyMat", 1., 1*g/mole, 1*g/cm3);

	// make laboratory
	G4Box* ROLabSolid = new G4Box("ROLabSolid", labX/2, labY/2, labZ/2);
	G4LogicalVolume* ROLabLV = new G4LogicalVolume(ROLabSolid, dummyMat, "ROLabLV");
	G4PVPlacement* ROLabPV = new G4PVPlacement(0, G4ThreeVector(), "ROLabPV", ROLabLV, 0, false, 0);

	const G4int numOfXVoxels = 15;
	const G4int numOfYVoxels = 15;
	const G4int numOfZVoxels = 800;

	// make detector
	G4Box* RODetSolid = new G4Box("ROTPCSolid", detX/2, detY/2, detZ/2);
	G4LogicalVolume* RODetLV = new G4LogicalVolume(RODetSolid, dummyMat, "RODetLV");
	G4VPhysicalVolume* RODetPV = new G4PVPlacement(0, detPosition, "RODetPV", RODetLV, ROLabPV, false, 0);

	// make voxel
	G4Box* RODetXVoxelSolid = new G4Box("RODetXVoxelSolid",
																			detX/numOfXVoxels/2,
																			detY/2,
																			detZ/2);
	G4LogicalVolume* RODetXVoxelLV = new G4LogicalVolume(RODetXVoxelSolid, dummyMat, "RODetXVoxelLV");
	G4VPhysicalVolume* RODetXVoxelPV = new G4PVReplica("RODetXVoxelPV",
																										 RODetXVoxelLV,
																										 RODetPV,
																										 kXAxis,
																										 numOfXVoxels,
																										 detX/numOfXVoxels);

	G4Box* RODetYVoxelSolid = new G4Box("RODetYVoxelSolid",
																			detX/numOfXVoxels/2,
																			detY/numOfYVoxels/2,
																			detZ/2);
	G4LogicalVolume* RODetYVoxelLV = new G4LogicalVolume(RODetYVoxelSolid, dummyMat, "RODetYVoxelLV");
	G4VPhysicalVolume* RODetYVoxelPV = new G4PVReplica("RODetYVoxelPV",
																										 RODetYVoxelLV,
																										 RODetXVoxelPV,
																										 kYAxis,
																										 numOfYVoxels,
																										 detY/numOfYVoxels);

	G4Box* RODetZVoxelSolid = new G4Box("RODetZVoxelSolid",
																			detX/numOfXVoxels/2,
																			detY/numOfYVoxels/2,
																			detZ/numOfZVoxels/2);
	G4LogicalVolume* RODetZVoxelLV = new G4LogicalVolume(RODetZVoxelSolid, dummyMat, "RODetZVoxelLV");
	new G4PVReplica("RODetZVoxelPV",
									RODetZVoxelLV,
									RODetYVoxelPV,
									kZAxis,
									numOfZVoxels,
									detZ/numOfZVoxels);

	DetDummySD *dummySD = new DetDummySD;
	RODetZVoxelLV -> SetSensitiveDetector(dummySD);

	return ROLabPV;
}
