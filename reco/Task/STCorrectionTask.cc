#include "STCorrectionTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include <iostream>
using namespace std;

ClassImp(STCorrectionTask)

STCorrectionTask::STCorrectionTask()
{

}

STCorrectionTask::~STCorrectionTask()
{
}

InitStatus STCorrectionTask::Init()
{
  if(STRecoTask::Init()==kERROR)
    return kERROR;
  //  cout<<"[STCorrectionTask] in task "<<endl;
  //  fCorrection = STCorrection();
  fCorrection = new STCorrection();
  
  fClusterArray = (TClonesArray*) fRootManager -> GetObject("STHitCluster");
  if (fClusterArray == nullptr) {
    LOG(ERROR) << "Cannot find Cluster array!" << FairLogger::endl;
    return kERROR;
  }

  fHitArray = (TClonesArray*) fRootManager -> GetObject("STHit");
  if (fHitArray == nullptr) {
    LOG(ERROR) << "Cannot find Hit array!" << FairLogger::endl;
    return kERROR;
  }

  return kSUCCESS;
}

void STCorrectionTask::Exec(Option_t *opt)
{

  //  fCorrection -> Test(fClusterArray,fHitArray);  
  if(fDesaturate)
    fCorrection -> Desaturate(fClusterArray);  
  //  LOG(INFO) << Space() << "STEmbedTrack "<< num_corr << FairLogger::endl;
}

void STCorrectionTask::SetDesaturation(Bool_t opt )
{
  fDesaturate = opt;
}
