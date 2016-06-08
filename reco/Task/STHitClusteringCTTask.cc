#include "STHitClusteringCTTask.hh"
#include "STEvent.hh"
#include "STClusterizerScan.hh"
#include "STClusterizerScan2.hh"
#include "STClusterizerLinearTrack.hh"
#include "STClusterizerCurveTrack.hh"
#include "STGlobal.hh"

// FairROOT classes
#include "FairRun.h"
#include "FairRuntimeDb.h"

ClassImp(STHitClusteringCTTask);

STHitClusteringCTTask::STHitClusteringCTTask()
: STRecoTask("Hit Clustering Task", 1, false)
{
}

STHitClusteringCTTask::STHitClusteringCTTask(Bool_t persistence)
: STRecoTask("Hit Clustering Task", 1, persistence)
{
}

STHitClusteringCTTask::~STHitClusteringCTTask()
{
}

void STHitClusteringCTTask::SetProximityCut(Double_t x, Double_t y, Double_t z)
{
  fSetProxCut = kTRUE;
  fXCut = x;
  fYCut = y;
  fZCut = z;
}

void STHitClusteringCTTask::SetSigmaCut(Double_t x, Double_t y, Double_t z)
{
  fSetSigmaCut = kTRUE;
  fSigmaXCut = x;
  fSigmaYCut = y;
  fSigmaZCut = z;
}

void STHitClusteringCTTask::SetEdgeCut(Double_t low, Double_t high)
{
  fSetEdgeCut = kTRUE;
  fXLowCut  = low;
  fXHighCut = high;
}

void STHitClusteringCTTask::SetClusterByLength(Bool_t flag) { fClusterByLength = flag; }

InitStatus STHitClusteringCTTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fTrackArray = (TClonesArray *) fRootManager -> GetObject("STCurveTrack");
  if (fTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STCurveTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fClusterArray = new TClonesArray("STHitCluster", 500);
  fRootManager -> Register("STHitCluster", "SpiRIT", fClusterArray, fIsPersistence);

  fClusterizer = new STClusterizerCurveTrack();
  if (fSetProxCut)  fClusterizer -> SetProximityCut(fXCut, fYCut, fZCut);
  if (fSetSigmaCut) fClusterizer -> SetSigmaCut(fSigmaXCut, fSigmaYCut, fSigmaZCut);
  if (fSetEdgeCut)  fClusterizer -> SetEdgeCut(fXLowCut, fXHighCut);
  if (fClusterByLength) fClusterizer -> SetClusteringOption(1);

  return kSUCCESS;
}

void
STHitClusteringCTTask::Exec(Option_t *opt)
{
  fClusterArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  fClusterizer -> AnalyzeTrack(fTrackArray, fClusterArray);

  LOG(INFO) << Space() << "STHitCluster " << fClusterArray -> GetEntriesFast() << FairLogger::endl;
}
