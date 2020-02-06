#include "STFilterTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TDatabasePDG.h"


ClassImp(STFilterTask);

STFilterTask::STFilterTask() : fMaxTheta(0)
{
  fLogger = FairLogger::GetLogger(); 
}

STFilterTask::~STFilterTask()
{}

InitStatus STFilterTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }
  if(fMaxTheta > 0)
    fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Tracks with polar angle > %f degrees will be removed", fMaxTheta));
  else 
  {
    fLogger -> Info(MESSAGE_ORIGIN, "No filter condition is set");
    return kERROR;
  }
  //fPDG = (STVectorI*) ioMan -> GetObject("PDG");
  fData = (TClonesArray*) ioMan -> GetObject("STData");
  return kSUCCESS;
}

void STFilterTask::SetParContainers()
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

void STFilterTask::Exec(Option_t *opt)
{
  auto data = (STData*) fData -> At(0);
  
  int npart = data -> multiplicity;
  for(int part = 0; part < npart; ++part)
  {
    if(data->vaMom[part].Theta()*TMath::RadToDeg() > fMaxTheta)
    {
      data->recoMom[part].SetXYZ(0,0,0);
      data->recoPosPOCA[part].SetXYZ(0,0,0);  
      data->recoPosTargetPlane[part].SetXYZ(0,0,0);  
      data->recodpoca[part].SetXYZ(0,0,0); 
      data->recoNRowClusters[part] = 0;
      data->recoCharge[part] = 0;
      data->recoCharge[part] = 0;
      data->recoEmbedTag[part] = false;
      data->recodedx[part] = 0;
 
      data->vaMom[part].SetXYZ(0,0,0);
      data->vaPosPOCA[part].SetXYZ(0,0,0);  
      data->vaPosTargetPlane[part].SetXYZ(0,0,0);  
      data->vadpoca[part].SetXYZ(0,0,0); 
      data->vaNRowClusters[part] = 0;
      data->vaCharge[part] = 0;
      data->vaCharge[part] = 0;
      data->vaEmbedTag[part] = false;
      data->vadedx[part] = 0;

    }
  }
}
