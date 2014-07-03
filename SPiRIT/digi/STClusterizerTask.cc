//---------------------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      STClusterizer reads in MCPoints and produces primary electrons
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//
// Rewritten for SPiRIT-TPC:
//      Genie Jhang     Korea Univ.
//      JungWoo Lee     Korea Univ.
//
//----------------------------------------------------------------------

// Fair & Collaborating class headers
#include "FairTask.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "LinearInterpolPolicy.h"

// SPiRIT-TPC headers
#include "STClusterizerTask.hh"
#include "STPrimaryCluster.hh"
#include "STPoint.hh"

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

STClusterizerTask()::STClusterizerTask()
: FairTask("SPiRIT Clusterizer"),
  fIsPersistant(kFalse)
  fIsSimpleChargeConversion(kFalse)
{
  fMCPointBranchName = "STPoint";
}

STClusterizerTask()::~STClusterizerTask()
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
  if(!db) Faltal("SetParContainers", "no runtime database");

  fPar = (STDigiPar*) db -> GetContainer("STDigiPar");
  if(!fPar) Faltal("SetParContainers", "STDigiPar not found");
}

void
STClusterizerTask::Exec(Option_t *opt)
{
  // Reset output array
  if(fPrimaryClusterArray == 0)
    Fatal("STClusterizerCluster::Exec)","No Primary Cluster Array");

  fPrimaryClusterArray -> Delete();

  Int_t numPoints = fMCPointArray -> GetEntries();
  if (numPoints < 2){
    Int_t entryNum = (FairRun::Instance()) -> GetEventHeader() -> GetMCEntryNumber();
    TString warning = Form("STClusterizerTask::Exec entryNum:%i ", entryNum);
    Warning(warning, "Not enough Hits in Tpc for Digitization (<2)");
    return;
  }
  
  if(fIsSimpleChargeConversion)	{
  	ChargeConversion();
  	return;
  }

  //!IsSimpleChargeConversion >>>

  STPoint* crntPoint; // current point
  STPoint* lastPoint; // last    point
           lastPoint = (STPoint*) fMCPointArray -> At(0);

  Int_t    iCluster = 0;        // counting index
  Double_t displacementCut = 1; // mm

  for(Int_t iPoint=1; iPoint<numPoints; iPoint++)
  {
    crntPoint = (STPoint*) fMCPointArray -> At(iPoint);

    TVector3 pointPosition;     
    TVector3 lastPointPosition; 

    crntPoint -> Position(pointPosition);
    lastPoint -> Position(lastPointPosition);

    TVector3 displacement = pointPosition - lastPointPosition;

    // check if two hits are close
    if(displacement.Mag()>displacementCut) 
    {
      lastPoint = crntPoint;
      continue;
    }

    // check if two hits lie on the same track
    if(crntPoint->GetTrackID()==lastPoint->GetTrackID()) 
    {
      Double_t energyLoss = crntPoint -> GetEnergyLoss()*1E9; //convert from GeV to eV
      
      // ***
      // Step 0: calculate the overall ammount of charge, produced
      if(energyLoss<0) 
      {
        Error("STClusterizerTask::Exec","Note: particle:: negative energy loss!");
        lastPoint=crntPoint;
        continue;
      }
      unsigned int qTotal = (unsigned int) floor(fabs( energyLoss/fGas->GetEIonize() )); // total charge
      unsigned int qCluster = 0; // cluster charge
      unsigned int numbCluster = 0; // number of clusters

      // ***
      // Step 1: Create Clusters
      while(qTotal>0) 
      {
        qCluster = fGas -> GetRandomCS(gRandom->Uniform()); // get random cluster size
        if(qCluster>qTotal) qCluster = qTotal; 
        qTotal -= qCluster;

        // create cluster
        Int_t size = fMCPointArray -> GetEntriesFast();
        STPrimaryCluster* primCluster 
          = new ((*fMCPointArray)[size]) STPrimaryCluster(crntPoint -> GetTime(),    // time
                                                          qCluster,                  // charge
                                                          TVector(0,0,0),            // primary position
                                                          crntPoint -> GetTrackID(), // trackID
                                                          iPoint,                    // point count number
                                                          crnt -> GetSecID());       // section ID
        primCluster -> setIndex(size);  
        numbCluster++;
      } // end of cluster creation

      // ***
      // Step 2: Distribute Clusters along track segment

      LinearInterpolPolicy().Interpolate(theLastPoint, point, fMCPointArray, icluster, ncluster); 
      // <- where does this come from?
      iCluster += numbCluster;
    } // end of checking same track
    lastPoint = crntPoint;
  }

  cout << "STClusterizerTask:: " << fMCPointArray -> GetEntriesFast() << " clusters created" << endl;

  return;
}

void
STClusterizerTask::ChargeConversion()
{
  // Mean energy to create ion-electron pair
  Float_t eIonize;
          eIonize = fGas -> GetEionize()*1.e-9; // ionization energy in [GeV]
  
  Int_t numPoints = fMCPointArray -> GetEntries();
  for(Int_t iPoint = 1; iPoint < numPont; iPoint++)
  {
  	STPoint *point = (STPoint *) fMCPointArray -> At(iPoint);
    //Do no clustering just convert energy deposition to ionisation
	
    //is this assuming some actual, valid process done by GEANT 
    //(e.g. vibration mode, ...) or do we lose something here (F.B.)??

  	if (point -> GetEnergyLoss() < fFirstIonizationPotential)
      continue;
       
    Int_t numElectrons = Int_t(floor(((point -> GetEnergyLoss()) - fFirstIonizationPotential)/w_ion)) + 1;
	  
    Int_t size = fPrimaryClusterArray -> GetEntries();
    STPrimaryCluster *cluster = new((*fPrimaryClusterArray)[size]) STPrimaryCluster(point -> GetTime(),
                                                                                    numElectron,
                                                                                    TVector3(point -> GetX(),
                                                                                             point -> GetY(),
                                                                                             point -> GetZ()),
                                                                                    point -> GetTrackID(),
                                                                                    iPoint,
                                                                                    point -> GetSecID());
    cluster -> setIndex(size);
  }
}
