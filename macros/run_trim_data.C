void run_trim_data
(
  int fRunNo,
  int fSplitNo,
  TString fPathToData = ""
)
{
  TString sRunNo   = TString::Itoa(fRunNo, 10);
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  TString beamDataPathWithFormat = "/mnt/spirit/rawdata/misc/Frozen_Information_For_SpiRIT_Analysis/Aug2019/BeamData/beam/beam_run%d.ridf.root";
  TString fBeamData = Form(beamDataPathWithFormat.Data(), fRunNo);

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  TString systemDB = "systemDB.csv";
  TString runDB = "runDB.csv";
  TString fSystemDB = spiritroot + "parameters/" + systemDB;
  TString fRunDB = spiritroot + "parameters/" + runDB;

  auto fParamSetter = new STParameters(fRunNo, fSystemDB, fRunDB);
  auto fParameterFile = fParamSetter -> GetParameterFile();
  auto fSystemID = fParamSetter -> GetSystemID();


  TString par = spiritroot+"parameters/ST.parameters.par";
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root"; 
  TString in = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco.*.conc.root";
  TString out = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".conc.trimmed.root";

  TString log = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".conc.trimmed.log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  TChain chain("spirit");
  // same file is added twice because the conc files may have either of those tree name
  chain.Add(in + "/cbmsim");
  chain.Add(in + "/spirit");
 
  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto reader = new STConcReaderTask();
  reader -> SetChain(&chain);

  auto eventFilter = new STFilterEventTask();
  switch(fSystemID)
  {
    case 124112: 
      eventFilter -> SetBeamFor124Star("../parameters/isotopesCutG124.root");
      eventFilter -> SetVertexCut(-17.84, -11.69);
      eventFilter -> SetVertexBDCCut(0.0974, 0.848*3, 0.689, 3*0.803);
      eventFilter -> SetMultiplicityCut(50, 100, 20);
      break;
    case 132124: 
      eventFilter -> SetBeamCut("../parameters/BeamCut.root", "Sn132"); 
      eventFilter -> SetVertexCut(-18.480, -11.165);
      eventFilter -> SetVertexBDCCut(2.69e-1, 3*0.988, 3.71e-1, 3*0.7532);
      eventFilter -> SetMultiplicityCut(50, 100, 20);
      break;
    case 112124: 
      eventFilter -> SetBeamCut("../parameters/BeamCut.root", "Sn112"); 
      eventFilter -> SetVertexCut(-16.944, -11.727);
      eventFilter -> SetVertexBDCCut(-1.55e-1, 3*0.832, -3.18, 3*0.997);
      eventFilter -> SetMultiplicityCut(50, 100, 20);
      break;
    case 108112: 
      eventFilter -> SetBeamCut("../parameters/BeamCut.root", "Sn108");
      eventFilter -> SetVertexCut(-18.480, -11.165);
      eventFilter -> SetVertexBDCCut(-9.25e-3, 3*0.933, -2.80478, 3*0.8424);
      eventFilter -> SetMultiplicityCut(50, 100, 20);
      break;
  }
  eventFilter -> SetVertexXYCut(-15, 15, -225, -185);
  eventFilter -> SetRejectBadEvents();

  auto bdcInfo = new STAddBDCInfoTask();
  bdcInfo -> SetRunNo(fRunNo);
  bdcInfo -> SetBeamFile(fBeamData);

  run -> AddTask(reader);
  run -> AddTask(eventFilter);
  run -> AddTask(bdcInfo);

  run -> Init();
  run -> Run(0, chain.GetEntries());

  cout << "Log    : " << log << endl;
  cout << "Input : " << in << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}
