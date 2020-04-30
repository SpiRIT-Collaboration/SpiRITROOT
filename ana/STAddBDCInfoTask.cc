#include "STAddBDCInfoTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TDatabasePDG.h"


ClassImp(STAddBDCInfoTask);

STAddBDCInfoTask::STAddBDCInfoTask()
{
  fLogger = FairLogger::GetLogger(); 
}

STAddBDCInfoTask::~STAddBDCInfoTask()
{}

InitStatus STAddBDCInfoTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }
  //fPDG = (STVectorI*) ioMan -> GetObject("PDG");

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fEventIDArr = (TClonesArray*) ioMan -> GetObject("EventID");
  fRunIDArr = (TClonesArray*) ioMan -> GetObject("RunID");

  fLogger -> Info(MESSAGE_ORIGIN, TString("Loading beam data from file") + fBeamFilename);
  fBeamFile = new TFile(fBeamFilename);
  fBeamTree = (TTree *) fBeamFile -> Get("TBeam");
  fBeamTree -> SetBranchAddress("z", &fZ);
  fBeamTree -> SetBranchAddress("aoq", &fAoQ);
  fBeamTree -> SetBranchAddress("beta37", &fBeta37);
  fBDCTree = (TTree *) fBeamFile -> Get("TBDC");
  fBDCTree -> SetBranchAddress("bdc1x", &fBDC1x);
  fBDCTree -> SetBranchAddress("bdc1y", &fBDC1y);
  fBDCTree -> SetBranchAddress("bdc2x", &fBDC2x);
  fBDCTree -> SetBranchAddress("bdc2y", &fBDC2y);
  fBDCTree -> SetBranchAddress("bdcax", &fBDCax);
  fBDCTree -> SetBranchAddress("bdcby", &fBDCby);

  fBeamEnergy = new STBeamEnergy();
  fBeamEnergy -> setBeam(fRunNo);
  fBDCProjection = new STBDCProjection(TString(gSystem -> Getenv("VMCWORKDIR")) + "/parameters/ReducedBMap.txt");
  fBDCProjection -> setBeam(fRunNo);
  
  return kSUCCESS;
}

void STAddBDCInfoTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void STAddBDCInfoTask::SetZtoProject(double peakZ, double sigma, double sigmaMultiple)
{
  fPeakZ = peakZ;
  fSigma = sigma;
  fSigmaMultiple = sigmaMultiple;
}

void STAddBDCInfoTask::Exec(Option_t *opt)
{
  fEventID = (((STVectorI*) fEventIDArr -> At(0))->fElements)[0];
  if(fRegisterID) static_cast<STVectorI*>(fRunIDArr -> At(0)) -> fElements[0] = fRunNo;
  auto data = (STData*) fData -> At(0);
  
  // code copied from GenfitVATask
  fBeamTree -> GetEntry(fEventID - 1);
  fBDCTree -> GetEntry(fEventID - 1);
  fBeamEnergy -> reset(fZ, fAoQ, fBeta37);

  Double_t E1 = fBeamEnergy -> getCorrectedEnergy();
  if (fZ > 0 && fZ < 75 && fAoQ > 1. && 
      fAoQ < 3 && fBDC1x > -999 && fBDC1y > -999 && 
      fBDC2x > -999 && fBDC2y > -999) 
  {
    auto vertex = data -> tpcVertex;
    Double_t ProjectedAtZ = -580.4 + (fPeakZ != -9999 ? fPeakZ : vertex.Z());    
    fBDCProjection -> ProjectParticle(fBDC2x, fBDC2y, -2160., 
                                      fBDCax, fBDCby, fZ, 
                                      E1, ProjectedAtZ, fBeamEnergy -> getMass());//-580.4,-583.904
    
    data -> projx = fBDCProjection -> getX();
    data -> projy = fBDCProjection -> getY();
    data -> proja = fBDCProjection -> getA();
    data -> projb = fBDCProjection -> getB();
    data -> beamEnergyTargetPlane = fBDCProjection -> getMeVu();
    data -> betaTargetPlane = fBDCProjection -> getBeta();
  }
  else
  {
    data -> projx = -999;
    data -> projy = -999;
    data -> proja = -999;
    data -> projb = -999;
    data -> beamEnergyTargetPlane = -999;
    data -> betaTargetPlane = -999;
  }
}
