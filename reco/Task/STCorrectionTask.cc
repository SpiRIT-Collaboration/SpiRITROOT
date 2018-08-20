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

  std::ifstream infile(fPRFcut_file.Data()); 
  if(!infile.good())
    {
      std::cout << "== [STCorrectionTask] PRF file does not Exist!" << std::endl;
      fPRFCheck = false;
    }
  
  fCorrection = new STCorrection();

  return kSUCCESS;
}

void STCorrectionTask::Exec(Option_t *opt)
{
  if(fDesaturate)
    {
      fCorrection -> Desaturate(fHitClusterArray);  
      LOG(INFO) << Space() << "STCorrection  Clusters Desaturated" << FairLogger::endl;
    }
  
  if(fPRFCheck)
    {
      fCorrection -> LoadPRFCut(fPRFcut_file);
      fCorrection -> CheckClusterPRF(fHitClusterArray,fHelixArray,fHitArray);  
      LOG(INFO) << Space() << "STCorrection  Check PRF of Clusters" << FairLogger::endl;
    }

}

void STCorrectionTask::SetDesaturation(Bool_t opt )
{
  fDesaturate = opt;
}

void STCorrectionTask::SetPRFCutFile(TString filename)
{
  fPRFcut_file = filename;
}
