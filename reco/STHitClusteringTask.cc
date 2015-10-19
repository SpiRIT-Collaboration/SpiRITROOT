/**
 * @brief Clustering hits processed by PSATask
 *
 * @author Genie Jhang (Korea University), original author
 * @author JungWoo Lee (Korea University)
 */

// SpiRITROOT classes
#include "STHitClusteringTask.hh"
#include "STEvent.hh"
#include "STClusterizerScan.hh"
#include "STClusterizerScan2.hh"

// FairROOT classes
#include "FairRun.h"
#include "FairRuntimeDb.h"

// ROOT classes
#include "TVector3.h"

// STL
#include <algorithm>

using std::vector;

ClassImp(STHitClusteringTask);

STHitClusteringTask::STHitClusteringTask()
:FairTask("SpiRIT Hit Clustering Task")
{
  fLogger = FairLogger::GetLogger();

  fSetProxCut  = kFALSE;
  fSetSigmaCut = kFALSE;
  fSetEdgeCut  = kFALSE;

  fClusterizerMode = kScan2;
}

STHitClusteringTask::~STHitClusteringTask()
{
}

void STHitClusteringTask::SetClusterizerMode(STClusterizerMode mode) { fClusterizerMode = mode; }
void STHitClusteringTask::SetProximityCut(Double_t x, Double_t y, Double_t z)
{
  fSetProxCut = kTRUE;
  fXCut = x;
  fYCut = y;
  fZCut = z;
}
void STHitClusteringTask::SetSigmaCut(Double_t x, Double_t y, Double_t z)
{
  fSetSigmaCut = kTRUE;
  fSigmaXCut = x;
  fSigmaYCut = y;
  fSigmaZCut = z;
}
void STHitClusteringTask::SetEdgeCut(Double_t low, Double_t high)
{
  fSetEdgeCut = kTRUE;
  fXLowCut  = low;
  fXHighCut = high;
}

STClusterizer* STHitClusteringTask::GetClusterizer() { return fClusterizer; }

InitStatus
STHitClusteringTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fEventArray = (TClonesArray *) ioMan -> GetObject("STEvent");
  if (fEventArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEvent array!");
    return kERROR;
  }

  fClusterizer = NULL;
  if (fClusterizerMode == kScan) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STClusterizerScan!");
    fClusterizer = new STClusterizerScan();
  } 
  else if (fClusterizerMode == kScan2) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use STClusterizerScan2!");
    fClusterizer = new STClusterizerScan2();
  }

  if (fClusterizer) {
    if (fSetProxCut)  fClusterizer -> SetProximityCut(fXCut, fYCut, fZCut);
    if (fSetSigmaCut) fClusterizer -> SetSigmaCut(fSigmaXCut, fSigmaYCut, fSigmaZCut);
    if (fSetEdgeCut)  fClusterizer -> SetEdgeCut(fXLowCut, fXHighCut);
  }

  fDriftLength = fPar -> GetDriftLength();

  return kSUCCESS;
}

void
STHitClusteringTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");
}

void
STHitClusteringTask::Exec(Option_t *opt)
{
  STEvent *event = (STEvent *) fEventArray -> At(0);

  if (event -> IsGood() == kFALSE) 
  {
    fLogger -> Info(MESSAGE_ORIGIN, Form("Event #%d : Bad event!", event -> GetEventID()));
    return;
  }

  if (fClusterizerMode != kX) 
  {
    fClusterizer -> Analyze(event);
    event -> SetIsClustered(kTRUE);

    fLogger -> Info(MESSAGE_ORIGIN, 
                  Form("Event #%d : Reconstructed %d clusters.",
                       event -> GetEventID(), event -> GetNumClusters()));
  } 
  else if (fClusterizerMode == kX)
  {
    vector<STHit> *hitArray = event -> GetHitArray();

    for (Int_t iHit = 0; iHit < hitArray -> size(); iHit++) 
    {
      STHit *hit = &(hitArray -> at(iHit));
      hit -> SetIsClustered(kTRUE);
      hit -> SetClusterID(event -> GetNumClusters());

      STHitCluster *cluster = new STHitCluster();
      cluster -> AddHit(hit);
      cluster -> SetClusterID(event -> GetNumClusters());

      event -> AddCluster(cluster);
      delete cluster;
    }

    event -> SetIsClustered(kTRUE);

    fLogger -> Info(MESSAGE_ORIGIN, 
                    Form("Event #%d : Reconstructed %d clusters.",
                         event -> GetEventID(), event -> GetNumClusters()));
  }
}
