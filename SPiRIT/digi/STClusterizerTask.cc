//---------------------------------------------------------------------
// Description:
//      Clusterizer task class source
//
//      STClusterizer reads in MCPoints and produces primary clusters
//
//      Input  : STMC
//      Output : STPrimaryCluster
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STClusterizerTask.hh"

// Fair & Collaborating class headers
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

// SPiRIT-TPC headers
#include "STPrimaryCluster.hh"
#include "STMCPoint.hh"

// ROOT headers
#include "TRandom.h"

// C/C++ headers
#include "cmath"
#include "iostream"

using std::cout;
using std::endl;
using std::floor;
using std::fabs;

ClassImp(STClusterizerTask)

STClusterizerTask::STClusterizerTask()
: FairTask("SPiRIT Clusterizer"),
  fIsPersistent(kFALSE)
{
  fMCPointBranchName = "STMCPoint";
}

STClusterizerTask::~STClusterizerTask()
{
}

InitStatus
STClusterizerTask::Init()
{
  // Get ROOT Manager
  FairRootManager* ioman = FairRootManager::Instance();

  if(!ioman) {
    Error("STClusterizerTask::Init", "RootManager not instantiated!");
    return kERROR;
  }
  
  // Get input collection
  fMCPointArray = (TClonesArray *) ioman -> GetObject(fMCPointBranchName);
  
  if(!fMCPointArray) {
    Error("STClusterizerTask::Init", "Point-array not found!");
    return kERROR;
  }
  
  // Create and register output array
  fPrimaryClusterArray = new TClonesArray("STPrimaryCluster"); 
  ioman -> Register("STPrimaryCluster", "ST", fPrimaryClusterArray, fIsPersistent);

  fGas = fPar -> GetGas();

  return kSUCCESS;
}

void STClusterizerTask::SetParContainers()
{
  cout << "STClusterizerTask::SetParContainers" << endl; //cout.flush()

  FairRun* run = FairRun::Instance();
  if(!run) Fatal("SetParContainers","No analysis run");

  FairRuntimeDb* db = run -> GetRuntimeDb();
  if(!db) Fatal("SetParContainers", "no runtime database");

  fPar = (STDigiPar*) db -> getContainer("STDigiPar");
  if(!fPar) Fatal("SetParContainers", "STDigiPar not found");
}

void
STClusterizerTask::Exec(Option_t *opt)
{
  if(fPrimaryClusterArray == 0)
    Fatal("STClusterizerCluster::Exec)","No Primary Cluster Array");
  fPrimaryClusterArray -> Delete();

  Int_t nPoints = fMCPointArray -> GetEntries();
  if (nPoints < 2){
    TString warning = Form("STClusterizerTask::Exec");
    Warning(warning, "Not enough Hits in Tpc for Digitization (<2)");
    return;
  }

  Double_t EIonize = fGas->GetEIonize();
           EIonize*= 1E-6;
  if(fTestMode) cout << "ionization energy : " << EIonize << endl;
  
  Int_t    iCluster = 0;        // cluster counting index
  Double_t displacementCut = 1; // [mm]

  STMCPoint* point; // STMCPoint

  for(Int_t iPoint=1; iPoint<nPoints; iPoint++)
  {
    point = (STMCPoint*) fMCPointArray -> At(iPoint);

    Double_t energyLoss = point -> GetEnergyLoss();
             energyLoss*= 1E9; //convert from GeV to eV
    if(fTestMode) cout << "energy loss    : " << energyLoss << " eV" << endl;
    if(energyLoss<0){
      Error("STClusterizerTask::Exec","Note: particle:: negative energy loss!");
      continue;
    }

    UInt_t qTotal   = (UInt_t) floor(fabs( energyLoss/EIonize )); // total charge [eV]
    UInt_t qCluster = 0; // cluster charge
    UInt_t nCluster = 0; // number of clusters in this point

    // Make random size clusters with total charge
    if(fTestMode) cout << "total charge   : " << qTotal << endl;
    while(qTotal>0) 
    {
      qCluster = fGas -> GetRandomCS(); // get random cluster size
      if(qCluster > qTotal) qCluster = qTotal; 
      if(fTestMode) cout << "cluster charge : " << qCluster << endl;
      qTotal -= qCluster;

      // create cluster
      Int_t size = fPrimaryClusterArray -> GetEntriesFast();
      STPrimaryCluster* primaryCluster 
        = new ((*fPrimaryClusterArray)[size]) STPrimaryCluster(qCluster,                  // charge
                                                               TVector3(point -> GetX(),  // primary cluster position
                                                                        point -> GetY(),
                                                                        point -> GetZ()),
                                                               point -> GetTime(),        // time
                                                               point -> GetTrackID(),     // trackID
                                                               iPoint);                   // point count number
      primaryCluster -> SetIndex(size);
      nCluster++;
    }
    iCluster += nCluster;
  }

  cout << "STClusterizerTask:: " << fPrimaryClusterArray -> GetEntriesFast() << " clusters created" << endl;

  return;
}
