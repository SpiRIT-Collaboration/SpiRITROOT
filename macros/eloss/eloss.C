#define ARRSIZE(x) (sizeof(x) / sizeof(x[0]))

#include <iostream>
#include <iomanip>

/** GLOBALS **/

const Int_t gSelection = 0;

const TString gTag = Form("VMCG4_%s", (gSelection == 0 ? "proton" : "alpha"));

const Bool_t gFlagRunMC     = 1;
const Bool_t gFlagRunAna    = 1;

//const TString gGeoFileName   = "spirit_v03.1.root";
const TString gGeoFileName   = "geomSPiRIT.root";
const TString gGenFileName   = "data/GEN_"+gTag+".sgen";
const TString gGeantMode     = "TGeant4";
const TString gMCFileName    = "data/SIM_"+gTag+".mc.root";
const TString gParFileName   = "data/SIM_"+gTag+".par.root";
const TString gSPiRITDir     = gSystem->Getenv("VMCWORKDIR");
const TString gGeomDir       = gSPiRITDir+"/geometry/";
const TString gConfDir       = gSPiRITDir+"/macros/eloss/gconfig/";
const TString gELossFileName = "data/eloss_"+gTag+".dat";

const Int_t gPDG[] = {2212, 1000020040};
const Double_t gMass[] = {0.938272, 3.728401};

const Double_t gKE[] = {0.01,0.011,0.012,0.013,0.014,0.015,0.016,0.017,0.018,
                        0.02,0.0225,0.025,0.0275,0.030,0.0325,0.035,0.0375,
                        0.04,0.045,0.05,0.055,0.06,0.065,0.07,0.08,0.09,0.1,
                        0.11,0.12,0.13,0.14,0.15,0.16,0.17,0.18,0.2,0.225,0.25,0.275,
                        0.3,0.325,0.35,0.375,0.4};

const Int_t gNEventPerEnergy = 1000;
const Int_t gNKE = ARRSIZE(gKE);
const Int_t gNRunKE = gNKE;
//const Int_t gNRunKE = gNKE;
const Int_t gNRunEvents = gNRunKE*gNEventPerEnergy;

const TString G4PhysicsList = "FTFP_BERT_EMY";
const TString G4Controls    = "stepLimiter";
//const TString G4Controls    = "stepLimiter+specialCuts+specialControls";

const Int_t Process_PAIR = 1; //!< pair production
const Int_t Process_COMP = 1; //!< Compton scattering
const Int_t Process_PHOT = 1; //!< photo electric effect
const Int_t Process_PFIS = 1; //!< photofission
const Int_t Process_DRAY = 0; //!< delta-ray
const Int_t Process_ANNI = 1; //!< annihilation
const Int_t Process_BREM = 1; //!< bremsstrahlung
const Int_t Process_HADR = 1; //!< hadronic process
const Int_t Process_MUNU = 1; //!< muon nuclear interaction
const Int_t Process_DCAY = 1; //!< decay
const Int_t Process_LOSS = 4; //!< energy loss
const Int_t Process_MULS = 1; //!< multiple scattering

const Double_t CUT_C  = 1.0E-4; //!< CUT* [GeV]
const Double_t CUT_B  = 1.0E4;  //!< BCUT* [GeV]
const Double_t CUT_E  = 1.0E-3; //!< CUTELE [GeV]
const Double_t CUT_P  = 1.0E4;  //!< PPCUTM [GeV]
const Double_t TOFMAX = 1.E10;  //!< TOFMAX [GeV]

const Int_t    CutLowEdge_eV   = 1; // eV
const Int_t    CutHighEdge_MeV = 1000;   // MeV
const Double_t RangeCuts       = 0.01; // mm
const Double_t RangeCutForElectron = 100; // km



/************************************************************************/
void eloss()
{
  CreateG4Configure();

  if(gFlagRunMC)     RunMC();
  if(gFlagRunAna)    RunAna();

  CreateLog();
}

/************************************************************************/
Double_t CalMomentum(Double_t e, Double_t m) { return sqrt((e+m)*(e+m)-m*m); }

/************************************************************************/
void RunMC()
{
  gSystem->Setenv("GEOMPATH",gGeomDir);
  gSystem->Setenv("CONFIG_DIR",gConfDir);

  /** Settings **/
  FairRunSim* run = new FairRunSim();
  FairRuntimeDb* rtdb = run->GetRuntimeDb();

  FairModule* cave = new FairCave("CAVE");
  cave->SetGeometryFileName("cave_vacuum.geo"); 
  FairDetector* spirit = new STDetector("STDetector", kTRUE);
  spirit->SetGeometryFileName(gGeoFileName);
  FairConstField* fMagField = new FairConstField();

  fMagField->SetField(0,0,0);
  fMagField->SetFieldRegion(-90,90,-50,50,-100,100);

  Double_t *listP = new Double_t[gNKE];
  for (Int_t iKE = 0; iKE < gNKE; iKE++)
    listP[iKE] = CalMomentum(gKE[iKE], gMass[gSelection]);

  STSimpleEventGenerator* gen = new STSimpleEventGenerator(gPDG[gSelection], gNKE, listP, gNEventPerEnergy, 0, -25, 1);
  FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
  primGen->AddGenerator(gen);

  /** Initialization & Run **/
  run->SetName(gGeantMode);
  run->SetOutputFile(gMCFileName);
  run->SetWriteRunInfoFile(kFALSE);
  run->SetMaterials("media.geo");
  run->AddModule(cave);
  run->AddModule(spirit);
//  run->SetField(fMagField);
  run->SetGenerator(primGen);
  run->Init();

  FairParRootFileIo* parOut = new FairParRootFileIo(kTRUE);
  parOut->open(gParFileName.Data());
  rtdb->setOutput(parOut);
  rtdb->saveOutput();
  rtdb->print();

  run->Run(gNRunEvents);
  delete run;
}


/************************************************************************/

void RunAna()
{
  std::ofstream oFile(gELossFileName);

  TClonesArray* pointArray;
  STMCPoint* point;

  TClonesArray* trackArray;
  STMCTrack* track;

  TFile* file = new TFile(gMCFileName,"read");
  TTree* tree = file->Get("cbmsim");
  tree->SetBranchAddress("STMCPoint",&pointArray);
  tree->SetBranchAddress("PrimaryTrack",&trackArray);

  TH1D* hist = new TH1D("dEdx","",2,0,0.1);
  for(Int_t iKE=0; iKE<gNRunKE; iKE++) {
    Double_t pPrimTrack = 0;
    Double_t kEPrimTrack = 0;
    for(Int_t iTracks=0; iTracks<gNEventPerEnergy; iTracks++) 
    {
      Double_t lMax=0;
      Double_t energyLossSum=0;

      Int_t iEvent = iTracks+iKE*gNEventPerEnergy;
      tree -> GetEntry(iEvent);

      track = (STMCTrack*) trackArray->At(0);
      pPrimTrack  = track->GetP();
      kEPrimTrack = track->GetEnergy() - track->GetMass();

      Int_t nPoints = pointArray->GetEntries();
      for(Int_t iPoint=0; iPoint<nPoints; iPoint++) 
      {
        point = (STMCPoint*) pointArray->At(iPoint);

        Int_t    parentID     = point -> GetTrackID();
        Double_t lPenetration = point -> GetLength()*10;
        Double_t energyLoss   = point -> GetEnergyLoss()*1000;

        if(parentID==0&&lMax<lPenetration) lMax=lPenetration;
        energyLossSum += energyLoss;

        if (lMax > 5.) break;
      }
      hist->Fill(energyLossSum/lMax);
      energyLossSum=0;
      lMax = 0;
    }
    Double_t dEdx=hist->GetMean();
    Double_t error=hist->GetMeanError();
    oFile << setw(20) << kEPrimTrack 
          << setw(20) << pPrimTrack 
          << setw(20) << dEdx 
          << setw(20) << error << endl;
    hist->Reset();
  }
}


/************************************************************************/

void CreateG4Configure()
{
  std::ofstream conf("gconfig/g4Config.dat");
  conf << G4PhysicsList.Data() << endl;
  conf << G4Controls.Data()    << endl;
  conf.close();

  std::ofstream cut("gconfig/SetCuts.dat");
  cut << Process_PAIR << endl;
  cut << Process_COMP << endl;
  cut << Process_PHOT << endl;
  cut << Process_PFIS << endl;
  cut << Process_DRAY << endl;
  cut << Process_ANNI << endl;
  cut << Process_BREM << endl;
  cut << Process_HADR << endl;
  cut << Process_MUNU << endl;
  cut << Process_DCAY << endl;
  cut << Process_LOSS << endl;
  cut << Process_MULS << endl;
  cut << CUT_C << endl;
  cut << CUT_B << endl;
  cut << CUT_E << endl;
  cut << CUT_P << endl;
  cut << TOFMAX << endl;
  cut.close();

  std::ofstream conf_in("gconfig/g4config.in");
  conf_in << "/cuts/setLowEdge  " << CutLowEdge_eV << " eV" << endl;
  conf_in << "/cuts/setHighEdge " << CutHighEdge_MeV << " MeV" << endl;
  conf_in << "/mcPhysics/rangeCuts " << RangeCuts << " mm" << endl;
  conf_in << "/mcPhysics/rangeCutForElectron " << RangeCutForElectron << " km" << endl;
  conf_in << "/mcDet/setIsUserMaxStep 1" << endl;
  conf_in.close();
}


/************************************************************************/

void CreateLog()
{
  std::ofstream log("data/LOG_"+gTag+".log");

  log << "Flag RunMC       " << gFlagRunMC       << endl;
  log << "Flag RunAna      " << gFlagRunAna      << endl;
  log << "----------------------------------------------------" << endl;
  log << "Geant Mode       " << gGeantMode       << endl;
  log << "Gen File Name    " << gGenFileName     << endl;
  log << "Geo File Name    " << gGeoFileName     << endl;
  log << "----------------------------------------------------" << endl;
  log << "PDG              " << gPDG[gSelection]  << endl;
  log << "Mass (GeV)       " << gMass[gSelection] << endl;
  log << "----------------------------------------------------" << endl;
  for(Int_t i=0; i<gNRunKE; i++)
  log << "KinE (GeV)       " << gKE[i]           << endl;
  log << "----------------------------------------------------" << endl;
  log << "NEvent per kinE  " << gNEventPerEnergy << endl;
  log << "Number of KinEs  " << gNRunKE          << endl;
  log << "NRun             " << gNRunEvents      << endl;
  log << "----------------------------------------------------" << endl;
  log << "Physics List     " << G4PhysicsList << endl;
  log << "G4 Constrols     " << G4Controls    << endl;
  log << "----------------------------------------------------" << endl;
  log << "Process PAIR     " << Process_PAIR << endl;
  log << "Process COMP     " << Process_COMP << endl;
  log << "Process PHOT     " << Process_PHOT << endl;
  log << "Process PFIS     " << Process_PFIS << endl;
  log << "Process DRAY     " << Process_DRAY << endl;
  log << "Process ANNI     " << Process_ANNI << endl;
  log << "Process BREM     " << Process_BREM << endl;
  log << "Process HADR     " << Process_HADR << endl;
  log << "Process MUNU     " << Process_MUNU << endl;
  log << "Process DCAY     " << Process_DCAY << endl;
  log << "Process LOSS     " << Process_LOSS << endl;
  log << "Process MULS     " << Process_MULS << endl;
  log << "----------------------------------------------------" << endl;
  log << "CUT CUT*         " << CUT_C << endl;
  log << "CUT B(D)CUT*,    " << CUT_B << endl;
  log << "CUT CUTELE       " << CUT_E << endl;
  log << "CUT PPCUTM       " << CUT_P << endl;
  log << "CUT TOFMAX       " << TOFMAX << endl;
  log << "----------------------------------------------------" << endl;
  log << "/cuts/setLowEdge     " << CutLowEdge_eV << " eV" << endl;
  log << "/cuts/setHighEdge    " << CutHighEdge_MeV << " MeV" << endl;
  log << "/mcPhysics/rangeCuts " << RangeCuts << " mm" << endl;
  log << "/mcPhysics/rangeCutForElectron " << RangeCutForElectron << " km" << endl;
  log << "/mcDet/setIsUserMaxStep 1" << endl;
  log << "----------------------------------------------------" << endl;
  log << "MC  File Name    " << gMCFileName      << endl;
  log << "Par File Name    " << gParFileName     << endl;

  log.close();
}
