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
//
//----------------------------------------------------------------------

#include "STClusterizerTask.hh"

ClassImp(STClusterizerTask)

STClusterizerTask()::STClusterizerTask()
:FairTask("SPiRIT Clusterizer")
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
}

void
STClusterizerTask::ChargeConversion()
{
  // Mean energy to create ion-electron pair
  Float_t w_ion = fgas -> W()*1.e-9;
  
  Int_t numPoints = fMCPointArray -> GetEntries();
  for(Int_t iPoint = 1; iPoint < numPont; iPoint++)
  {
  	STPoint *point = (STPoint *) fMCPointArray -> At(iPoint);
    //Do no clustering just convert energy deposition to ionisation
	
    //is this assuming some actual, valid process done by GEANT 
    //(e.g. vibration mode, ...) or do we lose something here (F.B.)??

  	if (point -> GetEnergyLoss() < fPoti)
      continue;
       
    Int_t numElectrons = Int_t(floor(((point -> GetEnergyLoss()) - fPoti)/w_ion)) + 1;
	  
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
