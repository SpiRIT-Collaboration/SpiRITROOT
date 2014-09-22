void run_digi
(TString mcFile      = "data/spirit_v2.root",
 TString mcParFile   = "data/spirit_params_v2.root",
 TString digiFile    = "data/spirit.digi.root",
 TString digiParFile = "ST.parameters.par")
{
  // __ Set Enviroment _________________________________

  TString sysDir = gSystem->Getenv("VMCWORKDIR");

  TString geomDir = sysDir + "/../SPiRIT/geometry";
  gSystem->Setenv("GEOMPATH",geomDir.Data());

  TString confDir = sysDir;
          confDir.ReplaceAll("/geometry","/gconfig");
  gSystem->Setenv("CONFIG_DIR",confDir.Data());

  TString digiParDir = sysDir;
          digiParDir += "/../SPiRIT/parameters/";
          digiParFile = digiParDir + digiParFile;


  // __ Digi output file _______________________________

  //TString digiFile=mcFile;
  //        digiFile.ReplaceAll(".root","_digi.root");


  // __ Run ____________________________________________

  FairRunAna* fRun = new FairRunAna();
              fRun -> SetInputFile(mcFile.Data());
              fRun -> SetOutputFile(digiFile.Data());


  // __ Parameter ______________________________________


  FairParRootFileIo*  mcParInput = new FairParRootFileIo();
                      mcParInput -> open(mcParFile);
  FairParAsciiFileIo* digiParInput = new FairParAsciiFileIo();
                      digiParInput -> open(digiParFile.Data());
  
  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
                 fDb -> setFirstInput(mcParInput);
                 fDb -> setSecondInput(digiParInput);

  // __ ST digi tasks___________________________________

  STDigiElectronTask* digi = new STDigiElectronTask();
                      digi -> SetPersistence(kTRUE);

  fRun -> AddTask(digi);

  // __ Init and run ___________________________________

  fRun -> Init();
  fRun -> Run(0,1);

  //fDb -> saveOutput();
  //fDb -> print();
}
