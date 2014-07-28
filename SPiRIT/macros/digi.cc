/*

// ROOT class headers
#include "TFile.h"

// Fair class headers
#include "FairRunSim.h"
#include "FairRuntimeDb.h"
#include "FairParRootFileIo.h"

*/

void digi(TString mcFile  = "data/test.mc_youngstest.root",
          TString mcParFile   = "data/params_youngstest.root",
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

  TString digiFile=mcFile;
          digiFile.ReplaceAll("mc","digi");


  // __ Run ____________________________________________

  FairRunAna* fRun = new FairRunAna();
              fRun -> SetInputFile(mcFile);
              fRun -> SetOutputFile(digiFile);


  // __ Parameter ______________________________________


  FairParRootFileIo*  mcParInput = new FairParRootFileIo();
                      mcParInput -> open(mcParFile);
  FairParAsciiFileIo* digiParInput = new FairParAsciiFileIo();
                      digiParInput -> open(digiParFile);
  
  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
                 fDb -> setFirstInput(mcParInput);
                 fDb -> setSecondInput(digiParInput);

  // __ ST digi tasks___________________________________

  STClusterizerTask* clusterizer = new STClusterizerTask();
                     clusterizer -> SetPersistence(kTRUE);
  fRun -> AddTask(clusterizer);


  // __ Init and run ___________________________________

  fRun -> Init();
  //fRun -> Run(0,10);
}
