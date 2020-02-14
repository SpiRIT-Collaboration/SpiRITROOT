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
  chain.Add(in);
 
  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto reader = new STConcReaderTask();
  reader -> SetChain(&chain);

  auto eventFilter = new STFilterEventTask();
  {
    TString beamName = "Sn108";
    if(fRunNo >= 2542 && fRunNo <= 2623) beamName = "Sn112";
    else if(fRunNo > 2623) beamName = "Sn132";
    eventFilter -> SetBeamCut("BeamCut.root", beamName);
  }
  eventFilter -> SetVertexCut(-18, -12);
  eventFilter -> SetMultiplicityCut(50, 100);

  auto bdcInfo = new STAddBDCInfoTask();
  bdcInfo -> SetRunNo(fRunNo);
  bdcInfo -> SetBeamFile(fBeamData);

  run -> AddTask(reader);
  run -> AddTask(eventFilter);
  run -> AddTask(bdcInfo);

  run -> Init();
  bdcInfo -> Register();
  run -> Run(0, chain.GetEntries());

  cout << "Log    : " << log << endl;
  cout << "Input : " << in << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}
