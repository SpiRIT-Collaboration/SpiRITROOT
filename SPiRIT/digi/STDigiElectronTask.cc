//---------------------------------------------------------------------
// Description:
//      DigiElectron task class source
//
//      STDigiElectronTask reads in MCPoints and produces primary clusters
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
  fIsPersistence(kFALSE)
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
  ioman -> Register("STDigitizedElectron", "ST", fDigitizedElectronArray, fIsPersistence);

  fRawEventArray = new TClonesArray("STRawEvent"); 
  ioman -> Register("STRawEvent", "SPiRIT", fRawEventArray, fIsPersistence);

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
  if(fDigitizedElectronArray == 0) Fatal("STDigiElectronTask::Exec)","No Digitized Electron Array");
  fDigitizedElectronArray -> Delete();

  if(fRawEventArray == 0) Fatal("STDigiElectronTask::Exec)","No Raw Event Array");
  fRawEventArray -> Delete();

  Int_t nPoints = fMCPointArray -> GetEntries();
  if(nPoints < 2){
    Warning("STDigiElectronTask::Exec", "Not enough Hits in Tpc for Digitization (<2)");
    return;
  }
  else cout << "[STDigiElectronTask] Number of MC points : " << nPoints << endl;

  TLorentzVector positionTimeMC;
  TLorentzVector positionTimeDrift;
  Double_t       zWire;
  Int_t          gain;

  InitializeRawEvent();
  wireResponse -> SetRawEvent(rawEvent, nTBs);

  for(Int_t iPoint=1; iPoint<nPoints; iPoint++) {
    cout << iPoint << " / " << nPoints << endl;
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
      gain              = wireResponse  -> FillPad(positionTimeDrift.X(),
                                                   positionTimeDrift.T());

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

  cout << "[STDigiElectronTask] Number of digi electron created : " 
       << fDigitizedElectronArray -> GetEntriesFast() << endl;

  wireResponse -> WriteHistogram();

  new ((*fRawEventArray)[0]) STRawEvent(rawEvent);

  delete rawEvent;

  return;
}

void
STDigiElectronTask::InitializeRawEvent()
{
  nTBs = fPar -> GetNumTbs(); // number of time buckets

  rawEvent = new STRawEvent();
  rawEvent -> SetName("rawEvent");
  rawEvent -> SetEventID(1);

  map = new STMap();
  map -> SetUAMap((fPar -> GetFile(0)).Data());
  map -> SetAGETMap((fPar -> GetFile(1)).Data());

  Int_t row, layer;

  for(Int_t iCoBo=0; iCoBo<12; iCoBo++){
    for(Int_t iAsAd=0; iAsAd<4; iAsAd++){
      for(Int_t iAGET=0; iAGET<4; iAGET++){
        for(Int_t iCh=0; iCh<68; iCh++){

          Bool_t isActive = map -> GetRowNLayer(iCoBo, iAsAd, iAGET, iCh, row, layer);
          if(!isActive) continue;

          STPad* pad = new STPad(row,layer);
                 pad -> SetPedestalSubtracted(kTRUE);

          for(int iTB=0; iTB<nTBs; iTB++){
            pad -> SetADC(iTB, 0);
          }

          rawEvent -> SetPad(pad);
          delete pad;
        }
      }
    }
  }

}
