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

  if(fPRFOpt == 0)
    {
      std::ifstream infile(fPRFcut_file.Data()); 
      if(!infile.good())
	{
	  std::cout << "== [STCorrectionTask] PRF file does not Exist!" << std::endl;
	  fPRFCheck = false;
	}
      else
	fPRFCheck = true;
    }
  
  std::ifstream infile(fExB_file.Data()); 
  if(!infile.good())
    {
      std::cout << "== [STCorrectionTask] ExB file does not Exist!" << std::endl;
      fExBShift = false;
    }
  else
    fExBShift = true;

  fCorrection = new STCorrection();

  if(fExBShift)
    fCorrection -> LoadExBShift(fExB_file);

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
  
  if(fPRFCheck && fPRFOpt == 0)
    {
      fCorrection -> LoadPRFCut(fPRFcut_file);
      fCorrection -> CheckClusterPRFOld(fHitClusterArray,fHelixArray,fHitArray);  
      LOG(INFO) << Space() << "STCorrection  Check PRF of Clusters OLD Cut (Only for testing!!!)" << FairLogger::endl;
    }
  
  if(fPRFOpt == 1)
    {
      fCorrection -> CheckClusterPRF(fHitClusterArray,fHelixArray,fHitArray);  
      LOG(INFO) << Space() << "STCorrection  Check PRF of Clusters" << FairLogger::endl;
    }

  if(fExBShift)
    {
      fCorrection -> CorrectExB(fHitClusterArray);
      LOG(INFO) << Space() << "STCorrection  Shift Clusters for ExB effect" << FairLogger::endl;
    }

  
}

void STCorrectionTask::SetPRFCutFile(TString filename)
{
  fPRFcut_file = filename;
}
