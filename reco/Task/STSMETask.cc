#include "STSMETask.hh"
#include "STParReader.hh"

#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"

ClassImp(STSMETask)

STSMETask::STSMETask()
: STRecoTask("System Manipulating Task", 1, false)
{
}

STSMETask::STSMETask(Bool_t persistence)
: STRecoTask("System Manipulating Task", 1, persistence)
{
}

STSMETask::~STSMETask()
{
}

void STSMETask::SetTrans(TVector3 trans) { fManipulator -> SetTrans(trans); }
void STSMETask::UseVertexFromParFile(Bool_t flag) { fVertexFlag = flag; } 

InitStatus STSMETask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fClusterArray = (TClonesArray *) fRootManager -> GetObject("STHitCluster");
  if (fClusterArray == nullptr) {
    LOG(INFO) << "Cannot find STHitCluster array!" << FairLogger::endl;
    return kERROR;
  }

  fSMClusterArray = new TClonesArray("STHitCluster", 500);
  fRootManager -> Register("STHitClusterSM", "SpiRIT", fSMClusterArray, fIsPersistence);

  fManipulator = new STSystemManipulator();

  if (fVertexFlag)
  {
    STRecoTask::SetParContainers();

    TString parName = fDigiPar -> GetTrackingParFileName();
    STParReader *tpar = new STParReader(parName);

    fManipulator -> SetTrans(TVector3(tpar -> GetDoublePar("XVertex"),
          tpar -> GetDoublePar("YVertex"),
          tpar -> GetDoublePar("ZVertex")));
  }

  return kSUCCESS;
}

void
STSMETask::Exec(Option_t *opt)
{
  fSMClusterArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  fManipulator -> Change(fClusterArray, fSMClusterArray);

  LOG(INFO) << Space() << "STHitCluster system manipulated" << FairLogger::endl;
}
