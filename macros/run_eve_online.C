#include <unistd.h>

void readEventList(TString eventListFile, map<Int_t, vector<Int_t> *> &events);

void run_eve_online
(
  Int_t fRunNo = 73,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 500,
  std::vector<Int_t> fSkipEventArray = {},
  TString fMCFile = "",
  TString fPathToData = "", 
  TString fSupplePath = "",
  Bool_t fIsFRIBDAQ = true
)
{
   cout << "running the macro" << endl;

  TString fSpiRITROOTPath = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  auto fParameterFile = "ST.parameters.Commissioning_201604.par";
  auto fTargetZ = -13.2;

  cout << "done with pars" << endl;

  Int_t start = fSplitNo * fNumEventsInSplit;

  TString sRunNo   = TString::Itoa(fRunNo, 10);
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  if (fPathToData.IsNull())
    fPathToData = fSpiRITROOTPath+"macros/data/";
  else 
    gSystem->mkdir(fPathToData.Data());
  
  TString version; {
    TString name = fSpiRITROOTPath + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  TString par = fSpiRITROOTPath+"parameters/"+fParameterFile;
  TString geo = fSpiRITROOTPath+"geometry/geomSpiRIT.man.root";
  TString fRawDataList = TString::Format("%s/list_run%04d.txt",(gSystem -> Getenv("PWD")), fRunNo);
  TString out = TString::Format("%srun%04d_s%d.reco.test.root", fPathToData.Data(),fRunNo,fSplitNo);
  TString log = TString::Format("%srun%04d_s%d.reco.test.log", fPathToData.Data(),fRunNo,fSplitNo);
  
  if (TString(gSystem -> Which(".", fRawDataList)).IsNull()) {
     cout << "data list not found" << endl;
     if(fIsFRIBDAQ)
        gSystem -> Exec("./createList_FRIBDAQ.sh "+sRunNo);
     else  
        gSystem -> Exec("./createList.sh "+sRunNo);
  }
     cout << "using data list " << fRawDataList << endl;

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  STEveManager *eve = new STEveManager();
  //eve -> SetInputFile(input);         // Set input file (string)
  eve -> SetParInputFile(par);  // Set parameter file (string)
  eve -> SetOutputFile(out);       // Set output file (string)
  eve -> SetBackgroundColor(kWhite);  // Set background color (Color_t) 
  eve -> SetGeomFile(geo);        // Set geometry file (string)
  eve -> SetVolumeTransparency(80);   // Set geometry transparency (integer, 0~100)
  eve -> SetViewerPoint(-0.7, 1.1);   // Set camera angle (theta, phi)
  eve -> SetIsNoFile(true);
  eve -> SetIsOnline(true);

  STEveDrawTask *draw = new STEveDrawTask();
  draw -> SetRendering("hit",        true);
  draw -> SetAttributes("hit", -1, -1, 4);
  draw -> SetRunNumber(fRunNo);

  STDecoderTask *decoder = new STDecoderTask();
  if(fIsFRIBDAQ)
    decoder -> SetUseFRIBDAQData();
  decoder -> SetUseSeparatedData(true);
  decoder -> SetPersistence(false);
  // By default, if SetUseGainCalibration(true) is called, reading gain calibration information from parameter file.
  //decoder -> SetUseGainCalibration(true);
  /* Manual calibration parameter setters. You need to provide both calibration root file and reference values to match.
   * Use default automatic one if you're not sure what you're doing. */
  // decoder -> SetGainCalibrationData("filename.root");
  // decoder -> SetGainReference(constant, linear, quadratic);
  decoder -> SetDataList(fRawDataList);
  decoder -> SetTbRange(30, 257); 
  // Low gain calibration. Don't forget you need to uncomment PSA part, too.
  //if (fUseGainMatching)
    //decoder -> SetGainMatchingData(fGainMatchingFile);
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
  //cout <<"Number of events " << fNumEventsInSplit << " starting at " << start <<endl;

  //decoder -> SetEventList(*events[fRunNo]);
  decoder -> SetEventID(start);
 
  auto preview = new STEventPreviewTask();
  preview -> SetSkippingEvents(fSkipEventArray);
  preview -> SetPersistence(false);
  //preview -> SetSelectingEvents(*events[fRunNo]);
  

  auto psa = new STPSAETask();
  psa -> SetPersistence(false);
  // In case pulse cut threshold need to be changed. (Default: 30)
  // psa -> SetThreshold(value);
  // By default, it uses all the layers.
  // psa -> SetLayerCut(-1, 112);
  // Pulse having long tail
  psa -> SetPulserData("pulser_117ns_50tb.dat");
  // Default: Rensheng's peak finding method 1. Finds small peaks
  // psa -> SetPSAPeakFindingOption(0);
  // Low gain calibration. Don't forget you need to uncomment decoder part, too.
  //if (fUseGainMatching)
    //psa -> SetGainMatchingData(fGainMatchingFile);
  // Statistically matching time jitter in different CoBos
  psa -> SetYOffsets(fSpiRITROOTPath + "parameters/yOffsetCalibration.dat");
  // This is used to match the TPC-Vertex_Y with the BDC_Y.
  //psa -> SetYPedestalOffset(fYPedestalOffset); // unit: mm

  eve -> AddTask(decoder);
  eve -> AddTask(preview);
  eve -> AddTask(psa);
  eve -> AddEveTask(draw);

  eve -> Init();
  
}

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
