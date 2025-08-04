#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void run_reco_sim2024(
  Int_t fRunNo = 1144,
    Int_t layer_cut = 112,
    TString fName = "test",
    Short_t tMF_Settings = 0,
    Int_t fNumEventsInRun = 10000,
    Int_t fNumEventsInSplit = 10000,
    Int_t fSplitNo = 0,
    bool simulateBeam = false,

    TString fOutName = "",
    // Int_t fRunNo = 0,
    Double_t fPSAThreshold = 30,
    TString fParameterFile = "ST.parameters.fullmc.par",  
    TString fMCFile = "",
    TString fPathToData = "data_sim/",
    Double_t fSheetCharge = 0,
    TString GainFile = "RelativeGain_Same.list"
  //Int_t fSplitNo = 1,
  //Int_t fNumEventsInSplit = 1000,
  //TString fOutForm = "reco.bulk",
  //TString fPathToData = "BulkReconstruction",
  //Bool_t fUseMeta = kTRUE
) {
  cout << "running the macro" << endl;		
  TString fSpiRITROOTPath = TString(gSystem->Getenv("VMCWORKDIR")) + "/";
  Bool_t fIsFRIBDAQ = true;
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  if (fPathToData.IsNull())
    fPathToData = "./data_sim/";
  TString version;
  {
    TString name = fSpiRITROOTPath + "VERSION";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  //=====Make test file to be deleted upon completion=====//
  auto testfile = TString::Format("%s/checkJobs/%04d_s%02d.txt", fPathToData.Data(), fRunNo, fSplitNo);
  std::ofstream testFile(testfile.Data()); testFile.close();
	
  //++++++++++++++++++++++++++++++++++//
  //+++++||FOR FAIRROOT RUN ANA||+++++//
  //++++++++++++++++++++++++++++++++++//

  //=====Set Input and Output Path=====//
  fPathToData = TString::Format("./%s/", fPathToData.Data()); // Data to be stored
  if (gSystem->AccessPathName(fPathToData.Data(), kFileExists)) {
    gSystem->mkdir(fPathToData.Data());
  }
  //TString out = TString::Format("%srun%04d_s%02d.%s.root", fPathToData.Data(), fRunNo, fSplitNo, fOutForm.Data());
  TString in1 = fPathToData + fName + ".digi.root";
  TString in2 = fPathToData + fName + ".mc.root";
  TString out = fPathToData + fOutName + "_s" + sSplitNo + ".layer_cut." + layer_cut + ".reco." + version + ".root";
  //=====End Input Output Path=====//

  //=====Set Parameter and Geometry File Paths=====//
  fParameterFile = "ST.parameters.2024Spring.par";
  TString par = fSpiRITROOTPath + "parameters/" + fParameterFile;
  TString geo = fSpiRITROOTPath + "geometry/geomSpiRIT.man.root";
  //=====End Parameter and Geometry File Paths=====//

  //+++++||END RUN ANA||+++++//

  //++++++++++++++++++++++++++++++//
  //+++++||FOR DECODER TASK||+++++//
  //++++++++++++++++++++++++++++++//

  //=====Set Gain Calibration=====//
  TString gainCalibFile=fSpiRITROOTPath + "parameters/gainCalibration_groundPlane_2024spring_threshold_10.root";
  double constant = 8.065E-2;
  double linear = 2.352E-3;
  double quadratic = -4.789E-8;
  //=====End Gain Calibration=====//

  //=====Set Start Event=====//
  Int_t start = fSplitNo * fNumEventsInSplit;
  if (start >= fNumEventsInRun) {cout<<"Past end of file, canceling split."<<endl; remove(testfile); return;}
  if (start + fNumEventsInSplit > fNumEventsInRun) {cout<<"Recalculating events in split."<<endl; fNumEventsInSplit = fNumEventsInRun - start;}
  //=====End Start Event=====//

  //+++++||END DECODER||+++++//

  //++++++++++++++++++++++++++//
  //+++++||FOR PSA TASK||+++++//
  //++++++++++++++++++++++++++//

  //=====Obtain run dependent Drift Velocity=====//

  double fDriftVelocity = 5.45;
  ifstream dvFile;
  if(fRunNo < 1600)
    dvFile.open("driftVelocity/driftVelocitySummary124Xe.txt");
  else
    dvFile.open("driftVelocity/driftVelocitySummary136Xe.txt");
  if(!dvFile.is_open()) {
    cout << "Error: Drift Velocity file not found!" << endl;
    cout << "Aborting" << endl;
    gApplication->Terminate();
  }
  int runN;
  double DV;
  int counts;
  bool dvGood = false;
  while(dvFile >> runN >> DV >> counts) {
    if(runN == fRunNo) {
      fDriftVelocity = DV;
      dvGood = true;
      continue;
    }
  }
  if(!dvGood) {
    cout << "Error: Run " << fRunNo << " is not in the drift velocity file!" << endl;
    cout << "Aborting" << endl;
    gApplication->Terminate();
  }
  //=====Finished Drift Velocity=====//

  //=====Obtain run dependent Y Offset (TB_0 != Ypos_0)=====//

  double fGGtb = 33;
  ifstream ggTBFile;
  if(fRunNo < 1600)
    ggTBFile.open("driftVelocity/GGD_tb_124Xe.txt");
  else
    ggTBFile.open("driftVelocity/GGD_tb_136Xe.txt");

  if(!ggTBFile.is_open()) {
    cout << "Error: GG TB file not found!" << endl;
    cout << "Aborting" << endl;
    gApplication->Terminate();
  }
  double TB;
  bool tbGood = false;
  while(ggTBFile >> runN >> TB) {
    if(runN == fRunNo) {
      fGGtb = TB;
      tbGood = true;
      continue;
    }
  }
  if(!tbGood) {
    cout << "Error: Run " << fRunNo << " is not in the GG TB file!" << endl;
    cout << "Aborting" << endl;
    gApplication->Terminate();
  }
  double fTBTime = 40;
  double fTbToYConv = -fTBTime * fDriftVelocity / 100;
  double fGGpos = fGGtb * fTbToYConv;
  double fYPedestalOffset = fGGpos - 16;
  cout << "Y Pedestal Offset: " << fYPedestalOffset << endl;
  cout << "Pad Plane TB: " << fYPedestalOffset / fTbToYConv;
  //=====Finished Y Offset=====//

  //+++++||END PSA||+++++//

  //++++++++++++++++++++++++++++++++++++//
  //+++++||FOR SPACE CHARGE TASK||++++++//
  //++++++++++++++++++++++++++++++++++++//

  //=====Set SAMURAI B-field offsets=====//
  auto fFieldOffsetX = -0.1794;
  auto fFieldOffsetY = -20.5502;
  auto fFieldOffsetZ = 58.0526;
  //=====End SAMURAI B-field offsets=====//

  //=====Set Space Charge Constants=====//
  double fSCslope = 0;
  double fSCinter = 0;

  if(fRunNo < 1600) {
    fSCslope = -1.1307e-11;
    fSCinter = 1.9819e-9;
  }
  else {
    fSCslope = -2.2799e-11;
    fSCinter = 2.9456e-9;
  }
  fSCslope = 0;
  fSCinter = 0;
  //=====End Space Charge Constants=====//

  //=====Set Rate File Location=====//
  auto fRateFile = TString::Format("RatesSup/run%04d.root", fRunNo);
  //=====End Rate File Location=====//

  //+++++||END SPACE CHARGE||+++++//

  //+++++++++++++++++++++++++++++//
  //+++++||FOR GENFIT TASK||+++++//
  //+++++++++++++++++++++++++++++//

  //=====Set Target Z Position=====//
  //auto fTargetZ = -19.66;
  auto fTargetZ = -21.0;
  if(fRunNo > 1599)
    fTargetZ = -30.8;
  //=====End Target Z Position=====//

  //+++++||END GENFIT||+++++//

  //+++++++++++++++++++++++++++++++++//
  //+++++||FOR GENFIT WITH BDC||+++++//
  //+++++++++++++++++++++++++++++++++//

  //=====Set BDC Offsets=====//
  auto fBDCOffsetX = -4.62; // Spring Value
  auto fBDCOffsetY = -181.8; // Same for Spring and Fall

  if(fRunNo > 1599) {
    fBDCOffsetX = -2.70; // Fall Value
  }
  //=====End BDC Offsets=====//


  //==================================================================//
  //========              Main Reconstruction                 ========//
  //==================================================================//

  //=====FAIRROOT setup=====//
  FairLogger *logger = FairLogger::GetLogger();

  auto parReader = new FairParAsciiFileIo();
  parReader->open(par);

  FairFileSource *inputFile = nullptr;
  inputFile = new FairFileSource(in1);
  bool has_in2 = false;
  {
    TFile file(in2);
    if (file.IsOpen())
      has_in2 = true;
  }
  if (has_in2)
    inputFile->AddFriend(in2);

  auto run = new FairRunAna();
  run->SetSource(inputFile);
  run->SetGeomFile(geo);
  run->SetOutputFile(out);
  run->GetRuntimeDb()->setSecondInput(parReader);
  //=====End FAIRROOT setup=====//

  //=====||Aux Header Task||=====//
  auto auxHead = new STAuxHeaderTask();
  auxHead->SetInputBranch("DigiAuxHeader");
  auxHead->SetAuxBranch("STAuxHeader");
  auxHead->SetPersistence();
  //=====||End Aux Header||=====//

  //=====||Event Preview Task||=====//
  auto preview = new STEventPreviewTask();
  //=====||End Preview||=====//

  //=====||Pulse Shape Analyzer Task||=====//
  auto psa = new STPSAETask();
  psa->SetPersistence(false); //Only set to true if hits are needed. Takes up a lot of space.
  //==Set Run Drift Velocity==//
  psa->SetDriftVelocity(fDriftVelocity);
  //==Set Pulser Signal==//
  psa->SetPulserData("pulser_117ns_50tb.dat");
  //==Set Y Offset for pad specific jitter==//
  psa -> SetYOffsets(fSpiRITROOTPath + "parameters/yOffsetCalibration.dat");
  //==Set Y Offset for TB_0 != Ypos_0 ==//
  psa -> SetYPedestalOffset(fYPedestalOffset); // unit: mm
  //=====||End PSA||=====//

  //=====||Helix Tracking Task||=====//
  auto helix = new STHelixTrackingTask();
  helix->SetPersistence(false);
  helix->SetClusterPersistence(false); //Only set true is clusters are needed. Space.
  //==Left, right, top and bottom sides cut==//
  helix->SetClusterCutLRTB(420, -420, -64, -522);
  //=====||End Helix||=====//

  //=====||Correction Task||=====//
  //==Correct for Saturation==//
  auto correct = new STCorrectionTask();
  //=====||End Correction||=====//

  //=====||Space Charge Task||=====//
  auto spaceCharge = new STSpaceChargeCorrectionTask();
  //==Set the SAMURAI B-field==//
  auto gfBField = STGFBField::GetInstance("samurai_field_map", "A", fFieldOffsetX, fFieldOffsetY, fFieldOffsetZ);   
  spaceCharge -> SetBField(gfBField -> GetFieldMap());
  //==Set drift parameters==//
  spaceCharge -> SetDriftParameters(-4.355e4, -2.18); // omega tau and mu of the Langevin equation
  //==Leakage Factor is 0 because of gating grid==//
  const double fLeakageChargeFactor = 0;
  //==Set initial sheet charge density. This will be changed on the first event.==//
  spaceCharge -> SetSheetChargeDensity(fSCinter, fLeakageChargeFactor * fSCinter);
  //==Set the beam projectile depending on run number==//
  if(fRunNo < 1600)
    spaceCharge -> SetProjectile(STSpaceCharge::Projectile::Xe124); // Spring
  else
    spaceCharge -> SetProjectile(STSpaceCharge::Projectile::Xe136); // Fall
  //==Tell it to actually do the thin==//
  spaceCharge -> SetElectronDrift(true); 
  //==Set the information for finding SC density from the "local rate"==//
  spaceCharge -> SetLocalRate(fSCslope, fSCinter, fRateFile);
  //=====||End Space Charge||=====//

  //=====||GenFit Task||=====//
  auto genfitPID = new STGenfitPIDTask();
  //==Set Target Z position in TPC frame for GenFit extrapolation==//
  genfitPID->SetTargetPlane(0, 0, fTargetZ); // unit: mm
  genfitPID->SetPersistence(true);
  genfitPID->SetListPersistence(true);
  //=====||End GenFit||=====//

  //=====||GenFit Task with BDC||=====//
  auto genfitVA = new STGenfitVATask();
  genfitVA->SetIs2024Data();
  genfitVA->SetPersistence(true);
  genfitVA->SetListPersistence(false);
  genfitVA->SetAuxBranch("STAuxHeader");
  //==Set the Beam Data file==//
  genfitVA->SetFixedVertex(0., -190.0, -21.0); // numbers are very different from the initial Vertex ones. Is it correct?
  //genfitVA->SetBeamFile(fBeamData);
  //==Set the BDC offset infromation==//
  //genfitVA->SetInformationForBDC(fRunNo, fBDCOffsetX, fBDCOffsetY, 0);
  //==Recalculate the vertex using refit tracks==//
  genfitVA->SetUseRave(true);
  //=====||End GenFit with BDC||=====//

  //=====||Add Tasks to Run Ana||=====//
  run->AddTask(auxHead);
  run->AddTask(preview);
  run->AddTask(psa);
  run->AddTask(helix);
  run->AddTask(correct);
  run->AddTask(spaceCharge);
  run->AddTask(genfitPID);
  run->AddTask(genfitVA);
  //=====||End Add Tasks||=====//

  //=====||Process the Run||=====//
  run->Init();
  cout << "processing run " << fRunNo << " with " << fNumEventsInSplit << " events." << endl;
  run->Run(0, fNumEventsInSplit);
  //=====||End Processing||=====//

  //=====||Finsh||=====//
  //cout << "Input  : " << fRawDataList << endl;
  cout << "Input  : " << in2 << endl;
  cout << "Output : " << out << endl;

  remove(testfile);
  gApplication->Terminate();
}
