#include <unistd.h>

//void readEventList(TString eventListFile, map<Int_t, vector<Int_t> *> &events);
void readEventList(TString eventListFile, vector<Int_t> &events);

void run_reco_experiment
(
  TString fMCFile = "",
  TString fOUTName = "",
  Int_t fRunNo = 2894,
  Int_t fNumEventsInRun = 500,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 500,
  TString fPathToData = "",
  TString fGCData = "/data/Q18393/production/calib/120fC_RUN1984-2017.root", // this parameter can be anything, but nothing.
  TString fGGData = "/data/Q18393/production/ggNoise/ggNoise_2349.root", //the different run will has different GG noise, for the accurate corresponding file, you can ask Genie.
  std::vector<Int_t> fSkipEventArray = {},
  TString fSupplePath = "/data/Q20393/rawdataSupplement",
  Double_t fPSAThreshold = 30,
  TString fParameterFile = "ST.parameters.PhysicsRuns_201707.par",
  Bool_t fUseMeta = kTRUE,
  double YPedestalOffset = +21.88,  // 132Sn: +21.88, 124Sn: +21.68, 112Sn: +22.98, 108Sn: +23.28, this value will move all the STHit YPedestalOffset, after applying this offset, the PosY of reconstructed TPC vertex will at -205.5mm, same with BDC.
  double BDC_Xoffset = -1.7,//-0.299, // 132Sn: -0.299, 124Sn: -0.609, 112Sn: -0.757, 108Sn: -0.706, this value will adjust the center of TPC vertex same with the center of BDC.
  double BDC_Yoffset = -210.0, //this value will transfer the BDC frame to TPC frame along Y direction.
  double BDC_Zoffset = 0,
  double charge_density = -1, // the magnitude of charge density used in space-charge correction
  bool fUseRunInfo = true // if enabled, it will look for run-by-run information in parameters/RunInfo.dat, and charge_density value will be ignored
)
{
  //  Int_t start = fSplitNo * fNumEventsInSplit;
  Int_t start = fSplitNo * 1000;
  if (start >= fNumEventsInRun) return;
  if (start + fNumEventsInSplit > fNumEventsInRun)
    fNumEventsInSplit = fNumEventsInRun - start;

  TString sRunNo   = TString::Itoa(fRunNo, 10);
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  else 
    gSystem->mkdir(fPathToData.Data());
  
  TString version; {
    TString name = spiritroot + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  TString par = spiritroot+"parameters/"+fParameterFile;
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString raw = TString(gSystem -> Getenv("PWD"))+"/list_run"+sRunNo+".txt";
  TString out = fPathToData+fOUTName+".embed.reco.root";
  TString log = fPathToData+fOUTName+".embed.reco.log";
  //  TString out = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".root";
  //  TString log = fPathToData+"run"+sRunNo+"_s"+sSplitNo+"."+version+".log";
  
  if (TString(gSystem -> Which(".", raw)).IsNull() && !fUseMeta)
    gSystem -> Exec("./createList.sh "+sRunNo);

  TString metaFile;
  if (fUseMeta) {
    raw = Form("%s/run_%04d/dataList.txt", fSupplePath.Data(), fRunNo);
    metaFile = Form("%s/run_%04d/metadataList.txt", fSupplePath.Data(), fRunNo);
  }

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  STDecoderTask *decoder = new STDecoderTask();
  decoder -> SetUseSeparatedData(true);
  decoder -> SetPersistence(false);
  if (fGCData.IsNull())
    decoder -> SetUseGainCalibration(false);
  else
    decoder -> SetUseGainCalibration(true);
  decoder -> SetGGNoiseData(fGGData);
  decoder -> SetDataList(raw);
  decoder -> SetEventID(start);
  decoder -> SetTbRange(30, 257); 
  // Low gain calibration. Don't forget you need to uncomment PSA part, too.
  decoder -> SetGainMatchingData(spiritroot + "parameters/RelativeGain.list");
  // Method to select events to reconstruct
  // Format of the input file:
  //        runid eventid
  //        runid eventid
  //        runid eventid
  //        runid eventid
  //map<Int_t, vector<Int_t> *> events;
  //string FileName_PiEvt = "./Pick_PiEvt/Sn108_PiEvt/";
  //FileName_PiEvt = FileName_PiEvt+"Sn108_Run"+fRunNo+"_PiEvt";
  //cout<<"Reading the Event list for the pion events : "<<FileName_PiEvt<<endl;
  //readEventList(FileName_PiEvt, events);
  cout <<"Number of events " << fNumEventsInSplit << " starting at " << start <<endl;

  vector<Int_t> events;
  readEventList(Form("/data/Q20393/isobe/20200811SingleIonMC/spiritroot.20200529/macros/makeEventSelection/eventlist/eventlist%d.txt", fRunNo), events);
  //  decoder -> SetEventList(events);

  //decoder -> SetEventList(*events[fRunNo]);
  if (fUseMeta) 
  {
    std::ifstream metalistFile(metaFile.Data());
    TString dataFileWithPath;
    for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
      dataFileWithPath.ReadLine(metalistFile);
      dataFileWithPath = Form("%s/run_%04d/%s", fSupplePath.Data(), fRunNo, dataFileWithPath.Data());
      decoder -> SetMetaData(dataFileWithPath, iCobo);
    }
  }

  auto embedTask = new STEmbedTask();
  embedTask -> SetEventID(start);
  embedTask -> SetEmbedFile(fMCFile);

  auto preview = new STEventPreviewTask();
  preview -> SetSkippingEvents(fSkipEventArray);
  preview -> SetPersistence(true);
  //  preview -> SetSelectingEvents(events);

  auto psa = new STPSAETask();
  psa -> SetPersistence(false);
  psa -> SetThreshold(fPSAThreshold);
  psa -> SetLayerCut(-1, 112);
  // Pulse having long tail
  psa -> SetPulserData("pulser_117ns_50tb.dat");
  // Rensheng's peak finding method (1).
  psa -> SetPSAPeakFindingOption(1);
  // Low gain calibration. Don't forget you need to uncomment decoder part, too.
  psa -> SetGainMatchingData(spiritroot + "parameters/RelativeGain.list");
  psa -> SetYOffsets(spiritroot + "parameters/yOffsetCalibration.dat");
  psa -> SetYPedestalOffset(YPedestalOffset); // unit: mm, this is used to match the TPC-Vertex_Y with the BDC_Y.
  
  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(false);
  helix -> SetClusterPersistence(false);
  // Left, right, top and bottom sides cut
//  helix -> SetClusterCutLRTB(420, -420, -64, -522);
  helix -> SetClusterCutLRTB(420, -420, -64+YPedestalOffset, -522+YPedestalOffset);
  // High density region cut
//  helix -> SetEllipsoidCut(TVector3(0, -260, -11.9084), TVector3(120, 100, 220), 5); // current use
  helix -> SetEllipsoidCut(TVector3(0, -260+YPedestalOffset, -11.9084), TVector3(120, 100, 220), 5); // current use
//  helix -> SetCylinderCut(TVector3(0, -226.06, -11.9084), 100, 100, 5);
//  helix -> SetSphereCut(TVector3(0, -226.06, -11.9084), 100, 5);
//  helix -> SetEllipsoidCut(TVector3(0, -206.34, -11.9084), TVector3(120, 55, 240), 5);
  // Changing clustering direction angle and margin. Default: 45 deg with 0 deg margin
  //helix -> SetClusteringAngleAndMargin(35., 3.);

  auto correct = new STCorrectionTask(); //Correct for saturation
  
  auto spaceCharge = new STSpaceChargeCorrectionTask();
  auto gfBField = STGFBField::GetInstance("samurai_field_map", "A", -0.1794, -20.5502, 58.0526);   
  spaceCharge -> SetBField(gfBField -> GetFieldMap());
  // this task will try to load space charge information from RunInfo.dat
  // Space charge correction will be disabled if the file is not present/Information of fRunNo is not found
  if(fUseRunInfo)
  {
    // space charge will only be enabled if run info contains information about the reco run
    if(spaceCharge -> SearchForRunPar(std::string(spiritroot.Data()) + "parameters/RunInfo.dat", fRunNo))
      spaceCharge -> SetElectronDrift(true);
    else
      spaceCharge -> SetElectronDrift(false); // disable space charge otherwise
  }else if(charge_density >= 0)
  {
    // If charge_density >= 0 and fUseRunInfo is disabled, you will need to configure everything manually
    spaceCharge -> SetDriftParameters(-4.355e4, -2.18); // omega tau and mu of the Langevin equation
    spaceCharge -> SetSheetChargeDensity(charge_density);  
    spaceCharge -> SetProjectile(STSpaceCharge::Projectile::Sn132); // Choose Projectile. Each of them have a slightly different beam path
    spaceCharge -> SetElectronDrift(true);
  }else spaceCharge -> SetElectronDrift(false);



  auto genfitPID = new STGenfitPIDTask();
  genfitPID -> SetFieldOffset(-0.1794, -20.5502, 58.0526); //unit: cm, which comes from Jon's measurement. It means the position of magnetic field in the TPC frame.
  genfitPID -> SetTargetPlane(0,-213.3,-13.4); // unit: mm, in the TPC frame. here the z position is used when Genfit do the extrapolation.
  genfitPID -> SetPersistence(true);
  genfitPID -> SetBDCFile("");
  //genfitPID -> SetConstantField();
  genfitPID -> SetListPersistence(true);
  // Removing shorter length tracklet by distance of adjacent clusters.
  //genfitPID -> SetMaxDCluster(60);
  
  auto genfitVA = new STGenfitVATask();
  genfitVA -> SetPersistence(true);
  //genfitVA -> SetConstantField();
  genfitVA -> SetListPersistence(true);
//  genfitVA -> SetBeamFile("");
//  genfitVA -> SetBeamFile(Form("/mnt/home/tsangchu/SpiRITROOT_develope/SpiRITROOT/BeamInfo/beam_run%d.ridf.root", fRunNo));
  genfitVA -> SetBeamFile(Form("/data/Q19393/BeamAnalysis/Sn132.raw/beam_run%d.ridf.root", fRunNo));
//  genfitVA -> SetInformationForBDC(fRunNo, /* xOffset */ -0.507, /* yOffset */ -227.013);
  genfitVA -> SetInformationForBDC(fRunNo,BDC_Xoffset,BDC_Yoffset,BDC_Zoffset);
  // Uncomment if you want to recalculate the vertex using refit tracks.
  genfitVA -> SetUseRave(true);
  genfitVA -> SetFieldOffset(-0.1794, -20.5502, 58.0526); //unit: cm, which comes from Jon's measurement. It means the position of magnetic field in the TPC frame.
  genfitVA -> Set_IsOption_BDCCorrection(0); 
  genfitVA -> Set_FileName_BDCCorrection_Theta_TargetPos("./BDCShift_Correction/f1_BDCCorrection_Theta_TargetPos.root");
  genfitVA -> SetZtoProject(-13.4, 1.7, 3); //(Double_t peakZ, Double_t sigma, Double_t sigmaMultiple), this function will project the BDC on the Target.
  
  auto embedCorr = new STEmbedCorrelatorTask();
  embedCorr -> SetPersistence(true);

  //  auto smallOutput = new STSmallOutputTask();
  //  smallOutput->SetOutputFile((fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".conc.root").Data());
  //smallOutput->SetRun(fRunNo);
    
  run -> AddTask(decoder);
  //  if(!fMCFile.IsNull()) run -> AddTask(embedTask);
  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(correct);
  run -> AddTask(spaceCharge);
  run -> AddTask(genfitPID);
  run -> AddTask(genfitVA);
  //  if(!fMCFile.IsNull()) run -> AddTask(embedCorr);
  //  run -> AddTask(smallOutput);
  
  auto outFile = FairRootManager::Instance() -> GetOutFile();
  auto recoHeader = new STRecoHeader("RecoHeader","");
  recoHeader -> SetPar("version", version);
  recoHeader -> SetPar("eventStart", start);
  recoHeader -> SetPar("numEvents", fNumEventsInSplit);
  recoHeader -> SetPar("parameter", fParameterFile);
  recoHeader -> SetPar("GCData", fGCData);
  recoHeader -> SetPar("GGData", fGGData);
  recoHeader -> Write("RecoHeader");

  run -> Init();
  helix -> GetTrackFinder() -> SetDefaultCutScale(2.5);
  helix -> GetTrackFinder() -> SetTrackWidthCutLimits(4, 10);
  helix -> GetTrackFinder() -> SetTrackHeightCutLimits(2, 4);

  run -> Run(0,fNumEventsInSplit);

  cout << "Log    : " << log << endl;
  cout << "Input  : " << raw << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();
}

/*
void readEventList(TString eventListFile, map<Int_t, vector<Int_t> *> &events) {
  vector<Int_t> *temp = new vector<Int_t>;

  ifstream eventList(eventListFile.Data());
  Int_t numEvents = 0;
  Int_t oldRunid = 0;
  Int_t runid, eventid;

  while (1) {
    eventList >> runid >> eventid;

    if (eventList.eof()) {
      events.insert(make_pair(oldRunid, temp));

      break;
    }
    
    if (oldRunid == 0)
      oldRunid = runid;

    if (oldRunid != runid) {
      events.insert(make_pair(oldRunid, temp));

      oldRunid = runid;
      temp = new vector<Int_t>;
    }

    temp -> push_back(eventid);
  }
}
*/

void readEventList(TString eventListFile, vector<Int_t> &events) {

  ifstream eventList(eventListFile.Data());
  Int_t numEvents = 0;
  Int_t oldRunid = 0;
  Int_t runid, eventid;
  Double_t vx, vy, vz;

  while (1) {
    eventList >> runid >> eventid >> vx >> vy >> vz;
    //    std::cout << runid << " " << eventid << std::endl;

    if (eventList.eof()) {
      break;
    }
    
    events.push_back(eventid);
  }

  std::cout << "event list from " << eventListFile << " " << events.size() << std::endl;
}

