//---------------------------------------------------------------------
// Description:
//      DigiElectron task class source
//
//      STDigiElectron reads in MCPoints and produces primary clusters
//
//      Input  : STMC
//      Output : STDigitizedElectron
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STDigiElectronTask.hh"

// Fair & Collaborating class headers
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

// SPiRIT-TPC headers
#include "STDigitizedElectron.hh"

// ROOT headers
#include "TRandom.h"

// C/C++ headers
#include "cmath"
#include "iostream"

using std::cout;
using std::endl;
using std::floor;
using std::fabs;

ClassImp(STDigiElectronTask);

STDigiElectronTask::STDigiElectronTask()
: FairTask("SPiRIT DigiElectron"),
  fIsPersistent(kFALSE)
{
  fMCPointBranchName = "STMCPoint";
}

STDigiElectronTask::~STDigiElectronTask()
{
}

InitStatus
STDigiElectronTask::Init()
{
  // Get ROOT Manager
  FairRootManager* ioman = FairRootManager::Instance();

  if(!ioman) {
    Error("STDigiElectronTask::Init", "RootManager not instantiated!");
    return kERROR;
  }
  
  // Get input collection
  fMCPointArray = (TClonesArray *) ioman -> GetObject(fMCPointBranchName);
  
  if(!fMCPointArray) {
    Error("STDigiElectronTask::Init", "Point-array not found!");
    return kERROR;
  }
  
  // Create and register output array
  fDigitizedElectronArray = new TClonesArray("STDigitizedElectron"); 
  ioman -> Register("STDigitizedElectron", "ST", fDigitizedElectronArray, fIsPersistent);

  fGas = fPar -> GetGas();
  EIonize = (fGas -> GetEIonize())*1E6; //convert from MeV to eV

  driftElectron = new STDriftElectron(fPar, fGas);
  wireResponse  = new STWireResponse(fGas);

  return kSUCCESS;
}

void STDigiElectronTask::SetParContainers()
{
  cout << "STDigiElectronTask::SetParContainers" << endl; //cout.flush()

  FairRun* run = FairRun::Instance();
  if(!run) Fatal("SetParContainers","No analysis run");

  FairRuntimeDb* db = run -> GetRuntimeDb();
  if(!db) Fatal("SetParContainers", "no runtime database");

  fPar = (STDigiPar*) db -> getContainer("STDigiPar");
  if(!fPar) Fatal("SetParContainers", "STDigiPar not found");
}

void
STDigiElectronTask::Exec(Option_t *opt)
{
  if(fDigitizedElectronArray == 0) Fatal("STDigiElectron::Exec)","No Digitized Electron Array");
  fDigitizedElectronArray -> Delete();

  Int_t nPoints = fMCPointArray -> GetEntries();
  if(nPoints < 2){
    Warning("STDigiElectronTask::Exec", "Not enough Hits in Tpc for Digitization (<2)");
    return;
  }
  else cout << "STDigiElectronTask : Number of MC pooints : " << nPoints << endl;

  TLorentzVector positionTimeMC;
  TLorentzVector positionTimeDrift;
  Double_t       zWire;
  Int_t          gain;

  for(Int_t iPoint=1; iPoint<nPoints; iPoint++) {
    MCPoint = (STMCPoint*) fMCPointArray -> At(iPoint);

    Double_t energyLoss = (MCPoint -> GetEnergyLoss())*1E9; //convert from [GeV] to [eV]
    if(energyLoss<0){
      Error("STDigiElectronTask::Exec","Note: particle:: negative energy loss!");
      continue;
    }

    positionTimeMC = TLorentzVector(MCPoint -> GetX(),
                                    MCPoint -> GetY(),
                                    MCPoint -> GetZ(),
                                    MCPoint -> GetTime());
    driftElectron ->  SetMCHit(positionTimeMC);

    UInt_t nElectron = (UInt_t) floor(fabs( energyLoss/EIonize )); // total charge [eV]
    for(Int_t iElectron=0; iElectron<nElectron; iElectron++){

      positionTimeDrift = driftElectron -> Drift();
      zWire             = wireResponse  -> FindZWire(positionTimeDrift.Z());
      gain              = wireResponse  -> FillPad(positionTimeDrift.X());

      Int_t size = fDigitizedElectronArray -> GetEntriesFast();
      STDigitizedElectron* electron 
        = new((*fDigitizedElectronArray)[size]) 
          STDigitizedElectron(positionTimeDrift.X(),
                              positionTimeDrift.Z(),
                              zWire,
                              positionTimeDrift.T(),
                              gain);
      electron -> SetIndex(size);
    }
  }

  cout << "STDigiElectronTask : Number of digi electron created : " 
       << fDigitizedElectronArray -> GetEntriesFast() << endl;

  //wireResponse  -> WriteHistogram();

  return;
}
