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

  fHelixArray = (TClonesArray*) fRootManager -> GetObject("STHelixTrack");
  if (fHelixArray == nullptr) {
    LOG(ERROR) << "Cannot find Helix array!" << FairLogger::endl;
    return kERROR;
  }

  fHitClusterArray = (TClonesArray*) fRootManager -> GetObject("STHitCluster");
  if (fHitClusterArray == nullptr) {
    LOG(ERROR) << "Cannot find Cluster array!" << FairLogger::endl;
    return kERROR;
  }

  fHitArray = (TClonesArray*) fRootManager -> GetObject("STHit");
  if (fHitArray == nullptr) {
    LOG(ERROR) << "Cannot find Hit array!" << FairLogger::endl;
    return kERROR;
  }

  fCorrection = new STCorrection();

  return kSUCCESS;
}

void STCorrectionTask::Exec(Option_t *opt)
{
  if(fDesaturate)
    {
      if(fSatOpt == 0)
	{
	  fCorrection -> Desaturate(fHitClusterArray);  
	  LOG(INFO) << Space() << "STCorrection  Clusters Desaturated (Pad PRF)" << FairLogger::endl;
	}
      else if(fSatOpt == 1)
	{
	  fCorrection -> Desaturate_byHelix(fHelixArray,fHitClusterArray);  
	  LOG(INFO) << Space() << "STCorrection  Clusters Desaturated (HelixPRF)" << FairLogger::endl;
	}
    }
  
}
