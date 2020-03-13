void run_create_trimmed_conc
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

  TString beamDataPathWithFormat = "/mnt/spirit/rawdata/misc/Frozen_Information_For_SpiRIT_Analysis/Aug2019/BeamData/beam/beam_run%d.ridf.root";
  TString fBeamData = Form(beamDataPathWithFormat.Data(), fRunNo);

  TString fSystemDB = fSpiRITROOTPath + "parameters/" + systemDB;
  TString fRunDB = fSpiRITROOTPath + "parameters/" + runDB;

  auto fParamSetter = new STParameters(fRunNo, fSystemDB, fRunDB);
  auto fParameterFile = fParamSetter -> GetParameterFile();
  auto fSystemID = fParamSetter -> GetSystemID();

  TString par = fSpiRITROOTPath+"parameters/"+fParameterFile;
  TString geo = fSpiRITROOTPath+"geometry/geomSpiRIT.man.root"; 
  TString in1 = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco.develop.1944.33821f0.root";
  TString out = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".conc.trimmed.root";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogScreenLevel("high");
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile(in1);
  run -> SetOutputFile(out);
  run -> SetGeomFile(geo);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto smallOutput = new STSmallOutputTask();
  smallOutput -> SetPersistence(true);
  smallOutput -> SetInPlace();

  auto eventFilter = new STFilterEventTask();
  switch(fSystemID)
  {
   case 124112: eventFilter -> SetBeamFor124Star("../parameters/isotopesCutG124.root"); break;
   case 132124: eventFilter -> SetBeamCut("BeamCut.root", "Sn132"); break;
   case 112124: eventFilter -> SetBeamCut("BeamCut.root", "Sn112"); break;
   case 108112: eventFilter -> SetBeamCut("BeamCut.root", "Sn108"); break;
  }
  eventFilter -> SetVertexCut(-18, -12);
  eventFilter -> SetMultiplicityCut(50, 100);

  auto bdcInfo = new STAddBDCInfoTask();
  bdcInfo -> SetRunNo(fRunNo);
  bdcInfo -> SetBeamFile(fBeamData);

  run -> AddTask(smallOutput);
  run -> AddTask(eventFilter);
  run -> AddTask(bdcInfo);

  run -> Init();
  bdcInfo -> Register();
  run -> Run();

  cout << "Input : " << in1 << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}

