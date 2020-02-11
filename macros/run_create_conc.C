void run_create_conc
(
  Int_t fRunNo = 2894,
  Int_t fSplitNo = 0,
  TString fPathToData = ""
)
{
  TString systemDB = "systemDB.csv";
  TString runDB = "runDB.csv";

  TString fSpiRITROOTPath = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = fSpiRITROOTPath+"macros/data/";
  TString version; {
    TString name = fSpiRITROOTPath + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  TString sRunNo   = TString::Itoa(fRunNo, 10);
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  TString fSystemDB = fSpiRITROOTPath + "parameters/" + systemDB;
  TString fRunDB = fSpiRITROOTPath + "parameters/" + runDB;

  auto fParamSetter = new STParameters(fRunNo, fSystemDB, fRunDB);
  auto fParameterFile = fParamSetter -> GetParameterFile();

  TString par = fSpiRITROOTPath+"parameters/"+fParameterFile;
  TString geo = fSpiRITROOTPath+"geometry/geomSpiRIT.man.root"; 
  TString in1 = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco.develop.1944.33821f0.root";
  TString out = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".temp.root";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile(in1);
  run -> SetOutputFile(out);
  run -> SetGeomFile(geo);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto smallOutput = new STSmallOutputTask();
  smallOutput -> SetOutputFile((fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".conc.root").Data());

  run -> AddTask(smallOutput);

  run -> Init();
  run -> Run();

  cout << "Input : " << in1 << endl;

  gApplication -> Terminate();

}

void run_create_conc(TString fName,
                     Int_t fSplitNo = 0,
                     TString fOutName = "",
                     TString fPathToData = "")
{
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  TString par = spiritroot+"parameters/ST.parameters.fullmc.par";
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString in = fPathToData+fName+"_s"+sSplitNo+".reco."+version+".root";
  TString out = fPathToData+fName+"_s"+sSplitNo+".reco."+version+".temp.root";
  TString realOut = fPathToData+fName+"_s"+sSplitNo+".reco."+version+".conc.root";
  if(!fOutName.IsNull())
    realOut = fPathToData+fOutName+"_s"+sSplitNo+".reco."+version+".conc.root";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile(in);
  run -> SetOutputFile(out);
  run -> SetGeomFile(geo);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto smallOutput = new STSmallOutputTask();
  smallOutput -> SetOutputFile(realOut.Data());

  run -> AddTask(smallOutput);

  run -> Init();
  run -> Run();

  cout << "Input : " << in << endl;
  cout << "Output : " << realOut << endl;

  gApplication -> Terminate();

}
