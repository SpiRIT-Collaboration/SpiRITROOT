#include "G4RunManager.hh"
#include "exampleDetectorConstruction.hh"
#include "examplePrimaryGeneratorAction.hh"
#include "exampleEventAction.hh"

#include "FTFP_BERT.hh"
#include "QGSP_BERT.hh"

#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4UImanager.hh"
#include "G4StepLimiterPhysics.hh"

#include "G4VisExecutive.hh"

int main(int argc, char ** argv)
{
  G4double kineticEnergy = 150;

  if(argc==3) kineticEnergy = atoi(argv[2]);

	G4RunManager *runManager = new G4RunManager();

	exampleDetectorConstruction* exampleDC = new exampleDetectorConstruction();
	runManager -> SetUserInitialization(exampleDC);

  G4VModularPhysicsList* physicsList = new QGSP_BERT;
  runManager->SetUserInitialization(physicsList);

	examplePrimaryGeneratorAction* examplePGA = new examplePrimaryGeneratorAction(kineticEnergy);
	runManager -> SetUserAction(examplePGA);

	exampleEventAction* exampleEA = new exampleEventAction(kineticEnergy);
	runManager -> SetUserAction(exampleEA);

	runManager -> Initialize();

	G4VisManager* visManager = new G4VisExecutive();
	visManager -> Initialize();

	if (argc == 1)
	{
		G4UIsession* session = new G4UIterminal(new G4UItcsh);
		session -> SessionStart();
		delete session;
	}
	else
	{
		G4String command = "/control/execute ";
		G4String fileName = argv[1];

		G4UImanager* UI = G4UImanager::GetUIpointer();
		UI -> ApplyCommand(command + fileName);
		
		G4UIsession* session = new G4UIterminal(new G4UItcsh);
		//session -> SessionStart();
		delete session;
	}

	delete runManager;

	return 0;
}
