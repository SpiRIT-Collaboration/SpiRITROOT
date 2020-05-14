#include <glob.h>

std::vector<std::string> glob(const char *pattern) {
    glob_t g;
    glob(pattern, GLOB_TILDE, nullptr, &g); // one should ensure glob returns 0!
    std::vector<std::string> filelist;
    filelist.reserve(g.gl_pathc);
    for (size_t i = 0; i < g.gl_pathc; ++i) {
        filelist.emplace_back(g.gl_pathv[i]);
    }
    globfree(&g);
    return filelist;
} 

void run_analysis_core(TString par, TString geo, TString out,
                       TString log, TChain& chain, int targetMass,
                       TString meta, TString pidFile, bool iterateMeta, 
                       EfficiencyFactory *effFactory, bool targetELoss=true,
                       TString unfoldFile = "")
{

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto reader = new STConcReaderTask();
  reader -> SetChain(&chain);

  //auto PIDCut = new STPIDCutTask();
  //PIDCut -> SetNPitch(1);
  //PIDCut -> SetNYaw(1);
  //PIDCut -> SetCutConditions(15, 15);
  //PIDCut -> SetCutFile("PIDCut_SimSn108.root");

  //auto unfold = new STUnfoldingTask();
  //unfold -> SetMomBins(100, 3700, 50, 25);
  //unfold -> SetThetaBins(0, 1.57, 30, 25);
  //unfold -> LoadMCData("cbmsim", "data/embed_dump/ImQMD_embedCorTriton2/*");
  //unfold -> SetCutConditions(15, 20);
  
  //auto PIDProb = new STPIDMachineLearningTask();
  //PIDProb -> SetChain(&chain);
  //PIDProb -> SetBufferSize(50000);
  //PIDProb -> SetModel("MLModelVoting", STAlgorithms::Voting);
  auto eventFilter = new STFilterEventTask();
  eventFilter -> SetMultiplicityCut(55, 100, 20);


  auto PIDProb = new STPIDProbTask();
  PIDProb -> SetMetaFile(meta.Data(), iterateMeta);
  PIDProb -> SetPIDFitFile(pidFile.Data());

  auto transform = new STTransformFrameTask();
  transform -> SetDoRotation(true);
  transform -> SetPersistence(true);
  transform -> SetTargetMass(targetMass);
  if(targetELoss)
  {
    transform -> SetTargetThickness(0.8);
    transform -> SetEnergyLossFile((targetMass == 112)? "../parameters/Sn108Sn112.txt": "../parameters/Sn132Sn124.txt");
  }

  auto efficiency = new STEfficiencyTask(effFactory);
  if(!unfoldFile.IsNull()) efficiency -> SetUnfoldingFile(unfoldFile);
  for(int pdg : STAnaParticleDB::SupportedPDG)
  {
    auto& settings = efficiency -> AccessSettings(pdg);
    settings.NClusters = 15; settings.DPoca = 15;
    settings.PhiCuts = {{160, 220},{0, 20}, {320, 360}};
    settings.ThetaMin = 0; settings.ThetaMax = 90; settings.NThetaBins = 15;
    auto &mBins = settings.NMomBins; auto &mMin = settings.MomMin; auto &mMax = settings.MomMax;
    auto &ptBins = settings.NPtBins; auto &ptMin = settings.PtMin; auto &ptMax = settings.PtMax;
    auto &CMzBins = settings.NCMzBins; auto &CMzMin = settings.CMzMin; auto &CMzMax = settings.CMzMax;
    mBins = 15; ptBins = 20; CMzBins = 30;
    if(pdg == 2212)      { mMin = 100;  mMax = 1500; ptMin = 0; ptMax = 1300; CMzMin = -1000; CMzMax = 1000; }
    if(pdg == 1000010020){ mMin = 200;  mMax = 2200; ptMin = 0; ptMax = 2000; CMzMin = -1300; CMzMax = 1300; }
    if(pdg == 1000010030){ mMin = 500;  mMax = 4500; ptMin = 0; ptMax = 2500; CMzMin = -2500; CMzMax = 2500; }
    if(pdg == 1000020030){ mMin = 100;  mMax = 3200; ptMin = 0; ptMax = 2500; CMzMin = -1900; CMzMax = 1900; }
    if(pdg == 1000020040){ mMin = 104; mMax = 4200; ptMin = 0; ptMax = 2500; CMzMin = -2500; CMzMax = 2500; }
    if(pdg == 1000020060){ mMin = 2000; mMax = 6000; ptMin = 0; ptMax = 1300; CMzMin = -1000; CMzMax = 1000; }
  }

  auto simpleGraphs = new STSimpleGraphsTask();

  run -> AddTask(reader);
  run -> AddTask(eventFilter);
  //run -> AddTask(filter);
  //run -> AddTask(PIDCut);
  run -> AddTask(PIDProb);
  //run -> AddTask(unfold);
  if(!iterateMeta)
  {
    run -> AddTask(transform);
    run -> AddTask(efficiency);
    run -> AddTask(simpleGraphs);
  }

  run -> Init();
  run -> Run(0, chain.GetEntries());

  cout << "Log    : " << log << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}

void run_analysis
(
  int fStartRunNo,
  int fEndRunNo,
  TString fOutName,
  TString fPathToData,
  int targetMass,
  TString metaFile,
  TString pidFile,
  bool iterateMeta = false,
  TString unfoldFile = "",
  bool effLab = true
)
{

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
  TString out;
  if(fOutName.IsNull())
  {
    out = fPathToData + "run";
    if(fEndRunNo > fStartRunNo)
      out += TString::Format("%d_%d", fStartRunNo, fEndRunNo);
    else out += TString::Format("%d", fStartRunNo);
  }
  else out = fPathToData + fOutName;

  TString log = out+"_ana.log";
  out += "_ana.root";

  TChain chain("cbmsim");
  for (int runNo = fStartRunNo; runNo <= fEndRunNo; ++runNo) {
    TString sRunNo = TString::Itoa(runNo, 10);
    chain.Add(fPathToData+"run"+sRunNo+"_s*.reco.*trimmed*.root/cbmsim");
    chain.Add(fPathToData+"run"+sRunNo+"_s*.reco.*conc*.root/spirit");
    chain.Add(fPathToData+"run"+sRunNo+"_s*.reco.*conc*.root/cbmsim");
    std::cout << "Reading from file " << fPathToData+"run"+sRunNo+"_s*.reco.*root" << std::endl;
  }

  {
    //auto origVerbosity = gErrorIgnoreLevel;
    //gErrorIgnoreLevel = kFatal;
    if(chain.GetEntries() == 0) throw std::runtime_error("No entries is being read from the file!");
    //gErrorIgnoreLevel = origVerbosity;
  }

  EfficiencyFactory *effFactory = nullptr;
  if(effLab)
  {
    auto effFactoryTmp = new EfficiencyFromConcFactory();
    //effFactoryTmp -> SetDataBaseForPDG(2212, "data/embed_dump/Feb20203/Run2899_embedNewProton/*conc*");
    //effFactoryTmp -> SetDataBaseForPDG(1000010020, "data/embed_dump/Feb20203/Run2899_embedNewDeuteron/*conc*");
    //effFactoryTmp -> SetDataBaseForPDG(1000010030, "data/embed_dump/Feb20203/Run2899_embedNewTriton/*conc*");
    //effFactoryTmp -> SetDataBaseForPDG(1000020030, "data/embed_dump/Feb20203/Run2899_embedNewHe3/*conc*");
    //effFactoryTmp -> SetDataBaseForPDG(1000020040, "data/embed_dump/Feb20203/Run2899_embedNewHe4/*conc*");
    //effFactoryTmp -> SetDataBaseForPDG(1000020060, "data/embed_dump/Feb20203/Run2899_embedNewHe6/*conc*");
    effFactoryTmp -> SetPhaseSpaceFactor(3); 
    effFactoryTmp -> SetDataBaseForPDG(2212, "data/embed_dump/Feb20203/Run2899KanekoNoSC_embedNewProton.root");
    effFactoryTmp -> SetDataBaseForPDG(1000010020, "data/embed_dump/Feb20203/Run2899KanekoNoSC_embedNewDeuteron.root");
    effFactoryTmp -> SetDataBaseForPDG(1000010030, "data/embed_dump/Feb20203/Run2899KanekoNoSC_embedNewTriton.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020030, "data/embed_dump/Feb20203/Run2899KanekoNoSC_embedNewHe3.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020040, "data/embed_dump/Feb20203/Run2899KanekoNoSC_embedNewHe4.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020060, "data/embed_dump/Feb20203/Run2899_embedNewHe6/*conc*");

    effFactory = effFactoryTmp;
  }
  else
  {
    auto effFactoryTmp = new EfficiencyInCMFactory();
    effFactoryTmp -> SetPhaseSpaceFactor(3); 
    effFactoryTmp -> SetDataBaseForPDG(2212, "data/embed_dump/MarchCM/Run2272Kaneko_CMProton.root");
    effFactoryTmp -> SetDataBaseForPDG(1000010020, "data/embed_dump/MarchCM/Run2272Kaneko_CMDeuteron.root");
    effFactoryTmp -> SetDataBaseForPDG(1000010030, "data/embed_dump/MarchCM/Run2272Kaneko_CMTriton.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020030, "data/embed_dump/MarchCM/Run2272Kaneko_CMHe3.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020040, "data/embed_dump/MarchCM/Run2272Kaneko_CMHe4.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020060, "data/embed_dump/MarchCM/Run2272_CutTPCCMHe6.root");
    effFactory = effFactoryTmp;
  }
  effFactory -> SetUpScalingFactor(10);

  run_analysis_core(par, geo, out, log, chain, targetMass, metaFile, pidFile, iterateMeta, effFactory, true, unfoldFile);
}

void run_analysis
(
  TString fName = "he4_lowmom",
  TString fOutName = "",
  TString fPathToData = "",
  int targetMass = 112,
  TString metaFile = "Meta_HalfPD.root",
  TString pidFile = "PIDSigma_HalfPD.root",
  bool iterateMeta = false,
  TString unfoldFile = "",
  bool effLab = true
)
{
  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  if(fOutName.IsNull()) fOutName = fName;

  TString par = spiritroot+"parameters/ST.parameters.fullmc.par";
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root"; 
  TString in1 = fPathToData+fName+".conc.root"; 
  TString out = fPathToData+fOutName+"_ana.root";
  TString log = fPathToData+fOutName+"_ana.log";

  TChain chain("spirit");
  for (const auto &filename : glob(in1.Data())) {
    chain.Add(filename.c_str());
  }

  EfficiencyFactory *effFactory = nullptr;
  if(effLab)
  {
    auto effFactoryTmp = new EfficiencyFromConcFactory();
    //effFactoryTmp -> SetDataBaseForPDG(2212, "data/embed_dump/Feb20203/SimSn108ProtonCompressed.root");
    //effFactoryTmp -> SetDataBaseForPDG(1000010020, "data/embed_dump/Feb20203/SimSn108DeuteronCompressed.root");
    //effFactoryTmp -> SetDataBaseForPDG(1000010030, "data/embed_dump/Feb20203/SimSn108TritonCompressed.root");
    //effFactoryTmp -> SetDataBaseForPDG(1000020030, "data/embed_dump/Feb20203/SimSn108He3Compressed.root");
    //effFactoryTmp -> SetDataBaseForPDG(1000020040, "data/embed_dump/Feb20203/SimSn108He4Compressed.root");
    //effFactoryTmp -> SetDataBaseForPDG(1000020060, "data/embed_dump/Feb20203/Run2899He6Compressed.root");
    effFactoryTmp -> SetDataBaseForPDG(2212, "data/embed_dump/Feb20203/SimSn132KanekoMult50_embedNewProton.root");
    effFactoryTmp -> SetDataBaseForPDG(1000010020, "data/embed_dump/Feb20203/SimSn132KanekoMult50_embedNewDeuteron.root");
    effFactoryTmp -> SetDataBaseForPDG(1000010030, "data/embed_dump/Feb20203/SimSn132KanekoMult50_embedNewTriton.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020030, "data/embed_dump/Feb20203/SimSn132KanekoMult50_embedNewHe3.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020040, "data/embed_dump/Feb20203/SimSn132KanekoMult50_embedNewHe4.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020060, "data/embed_dump/Feb20203/Run2899_embedNewHe6/*s0*conc*");
    effFactory = effFactoryTmp;
  }
  else
  {
    auto effFactoryTmp = new EfficiencyInCMFactory();
    effFactoryTmp -> SetPhaseSpaceFactor(3); 
    effFactoryTmp -> SetDataBaseForPDG(2212, "data/embed_dump/MarchCM/SimSn132KanekoMult50_CMProton.root");
    effFactoryTmp -> SetDataBaseForPDG(1000010020, "data/embed_dump/MarchCM/SimSn132KanekoMult50_CMDeuteron.root");
    effFactoryTmp -> SetDataBaseForPDG(1000010030, "data/embed_dump/MarchCM/SimSn132KanekoMult50_CMTriton.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020030, "data/embed_dump/MarchCM/SimSn132KanekoMult50_CMHe3.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020040, "data/embed_dump/MarchCM/SimSn132KanekoMult50_CMHe4.root");
    effFactoryTmp -> SetDataBaseForPDG(1000020060, "data/embed_dump/MarchCM/SimSn108_CMHe6.root");
    effFactory = effFactoryTmp;
  }
  effFactory -> SetUpScalingFactor(10);

  run_analysis_core(par, geo, out, log, chain, targetMass, metaFile, pidFile, iterateMeta, effFactory, false, unfoldFile);
}


