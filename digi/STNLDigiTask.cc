#include "STNLDigiTask.hh"
#include "STDigiPar.hh"
#include "STHit.hh"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>

// Root class headers
#include "TString.h"

STNLDigiTask::STNLDigiTask()
:FairTask("STNLDigiTask"), fEventID(0)
{
}

STNLDigiTask::~STNLDigiTask()
{
}

void 
STNLDigiTask::SetParContainers()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana -> GetRuntimeDb();
  auto par = (STDigiPar*) rtdb -> getContainer("STDigiPar");
}

InitStatus 
STNLDigiTask::Init()
{
  FairRootManager* ioman = FairRootManager::Instance();

  fMCPointArray = (TClonesArray*) ioman -> GetObject("NLMCPoint");

  fNLHitArray = new TClonesArray("STHit");
  ioman -> Register("NLHit","ST",fNLHitArray,true);
  
  return kSUCCESS;
}

void 
STNLDigiTask::Exec(Option_t* option)
{
  fNLHitArray -> Clear("C");

  Int_t numMCPoints = fMCPointArray -> GetEntries();

  if(numMCPoints<2) {
    LOG(WARNING) << "Not enough hits for digitization! ("<<numMCPoints<<"<2)" << FairLogger::endl;
    ++fEventID;
    return;
  }
  else
    LOG(INFO) << "Number of mc points: " << numMCPoints << FairLogger::endl;

  for(Int_t iPoint=0; iPoint<numMCPoints; iPoint++)
  {
    auto point = (STMCPoint*) fMCPointArray -> At(iPoint);

    auto t = point -> GetTime();
    auto e = point -> GetEnergyLoss();
    auto d = point -> GetDetectorID();

    TVector3 gpos = 10 * TVector3(point -> GetX(), point -> GetY(), point -> GetZ());
    auto local = GetNLLocalPos(gpos);

    auto hit = (STHit *) fNLHitArray -> ConstructedAt(iPoint);
    hit -> SetPosition(local);
    hit -> SetCharge(e);
    hit -> SetTb(t);
  }

  return;
}

void STNLDigiTask::SetHitPersistence(Bool_t value) { fIsHitPersistence = value; }
void STNLDigiTask::SetHitClusterPersistence(Bool_t value) { fIsHitClusterPersistence = value; }

TVector3 STNLDigiTask::GetNLGlobalPos(TVector3 local)
{
  TVector3 global = local;
  global.RotateY( fRotYNeuland_rad );
  global += TVector3(fOffxNeuland, fOffyNeuland, fOffzNeuland);

  return global;
}

TVector3 STNLDigiTask::GetNLLocalPos(TVector3 global)
{
  TVector3 local = global;
  local -= TVector3(fOffxNeuland, fOffyNeuland, fOffzNeuland);
  local.RotateY( -fRotYNeuland_rad );

  return local;
}

ClassImp(STNLDigiTask);
