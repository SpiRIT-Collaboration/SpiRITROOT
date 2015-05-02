void Config()
{
/// The second argument in the constructor selects physics list:
/// - emStandard         - standard em physics (default)
/// - emStandard+optical - standard em physics + optical physics
/// - XYZ                - selected hadron physics list ( XYZ = LHEP, QGSP, ...)
/// - XYZ+optical        - selected hadron physics list + optical physics
///
/// The third argument activates the special processes in the TG4SpecialPhysicsList,
/// which implement VMC features:
/// - stepLimiter       - step limiter (default) 
/// - specialCuts       - VMC cuts
/// - specialControls   - VMC controls for activation/inactivation selected processes
/// - stackPopper       - stackPopper process
/// When more than one options are selected, they should be separated with '+'
/// character: eg. stepLimit+specialCuts.

  TString configDir(gSystem->Getenv("VMCWORKDIR"));

  ifstream conf(configDir+"/macros/eloss/gconfig/g4Config.dat");
  TString physicsList; conf>>physicsList;
  TString control; conf>>control;
  conf.close();

  TG4RunConfiguration* runConfiguration 
    = new TG4RunConfiguration("geomRoot", physicsList, control);

  TGeant4* geant4 
    = new TGeant4("TGeant4", "The Geant4 Monte Carlo", runConfiguration);

  STStack *stack = new STStack(1000); 
  geant4->SetStack(stack);

  if(FairRunSim::Instance()->IsExtDecayer()){
    TVirtualMCDecayer* decayer = TPythia6Decayer::Instance();
    geant4->SetExternalDecayer(decayer);
  }

  configm1 = configDir + "/macros/eloss/gconfig/g4config.in";
  cout << " -I g4Config() using g4conf  macro: " << configm1 << endl;

  geant4->SetMaxNStep(1E5);  // default is 30000
  geant4->ProcessGeantMacro(configm1.Data());
}
