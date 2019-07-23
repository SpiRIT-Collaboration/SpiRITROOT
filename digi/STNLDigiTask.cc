#include "STNLDigiTask.hh"
#include "STDigiPar.hh"
#include "STNLHit.hh"
#include "STChannelBar.hh"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

// Root class headers
#include "TString.h"
#include "TCollection.h"

STNLDigiTask::STNLDigiTask(TString name)
:FairTask("STNLDigiTask"), fEventID(-1)
{
}

STNLDigiTask::~STNLDigiTask()
{
}

void 
STNLDigiTask::SetParContainers()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana -> GetRuntimeDb();
  auto par = (STDigiPar*) rtdb -> getContainer("STDigiPar");
}

InitStatus 
STNLDigiTask::Init()
{
  FairRootManager* ioman = FairRootManager::Instance();

  // input

  fMCPointArray = (TClonesArray*) ioman -> GetObject("NLMCPoint");

  // output

  fBarArray = new TClonesArray("STChannelBar");
  ioman -> Register("Bars","ST",fBarArray,true);

  fNLHitArray = new TClonesArray("STNLHit");
  ioman -> Register("NLHit","ST",fNLHitArray,true);

  fNLHitClusterArray = new TClonesArray("STNLHit");
  ioman -> Register("NLHitCluster","ST",fNLHitClusterArray,true);

  //

  fNL = STNeuLAND::GetNeuLAND();
  
  return kSUCCESS;
}

void 
STNLDigiTask::Exec(Option_t* option)
{
  ++fEventID;
  TString digiName = fName + Form("_%d",fEventID);

  fBarArray -> Clear("C");
  fNLHitArray -> Clear("C");
  fNLHitClusterArray -> Clear("C");

  Int_t numMCPoints = fMCPointArray -> GetEntries();

  if(numMCPoints<2) {
    LOG(INFO) << "  Event_" << fEventID << " : Not enough hits for digitization! ("<< numMCPoints << " < 2)" << FairLogger::endl;
    return;
  }

  STChannelBar *bar = nullptr;

  for(Int_t iPoint=0; iPoint<numMCPoints; iPoint++)
  {
    auto point = (STMCPoint*) fMCPointArray -> At(iPoint);

    auto t = point -> GetTime();
    auto e = point -> GetEnergyLoss();
    auto d = point -> GetDetectorID();

    TVector3 gpos = 10 * TVector3(point -> GetX(), point -> GetY(), point -> GetZ());
    auto local = fNL -> LocalPos(gpos);

    bar = (STChannelBar *) fBarArray -> ConstructedAt(d-4000);
    if (bar -> GetChannelID() < 0) {
      // name, id, layer, row
      // x_or_y, local_pos, 
      // num_tdc_bins, bar_length, attenuation_length
      // pulse_decay_time, pulse_rise_time, time_err,
      // effective_speed_of_light
      bar -> SetBar(digiName,
          d, fNL->GetLayer(d), fNL->GetRow(d),
          fNL->IsAlongXNotY(d), fNL->GetBarLocalPosition(d),
          100, 2500, 1250,
          2.1, 0.1, 0.0,
          140.);
    }
    bar -> Fill(local, e);
  }

  fBarArray -> Compress();

  // reconstruct neuland hits

  vector<STNLHit *> hitArray;

  TIter itBars(fBarArray);
  Int_t countHits = 0;
  STNLHit *hit = nullptr;
  while ((bar = (STChannelBar *) itBars())) {
    hit = (STNLHit *) fNLHitArray -> ConstructedAt(countHits);

    auto position = bar -> FindHit(0.00001);
    hit -> SetHitID(countHits);
    hit -> SetClusterID(bar -> GetChannelID());
    hit -> SetPosition(position);
    hit -> SetCharge(bar -> GetChargeA());

    hitArray.push_back(hit);

    ++countHits;
  }

  // reconstruct neuland hit clusters (neuland track)

  sort(hitArray.begin(), hitArray.end(), STNLHitSortZInv());

  Double_t dx_cut =  75.; // position difference cut : 7.5 cm = 75 mm
  Double_t dt_cut = 140.; // time cut : 1 ns = 14 cm  = 140 mm
  Double_t rs_cut = .5*dx_cut; // residual cut

  Int_t countHitClusters = 0;

  vector<STNLHit *> singleHitArray;

  // start with largest z hit : hitReferece
  // compare with all other hits : hitCandidate

  while(1)
  {
    if (hitArray.empty())
      break;

    auto hitReference = hitArray.back();
    auto posReference = hitReference -> GetPosition();
    hitArray.pop_back();

    STNLHit *cluster = nullptr;

    Int_t numHits2 = hitArray.size();
    for (auto idxHit2=numHits2-1; idxHit2>=0; --idxHit2)
    {
      auto hitCandidate = hitArray.at(idxHit2);
      auto posCandidate = hitCandidate -> GetPosition();

      // =======================================================================
      // check closest distance dx to hitReference of the all hits in cluster
      // =======================================================================

      Double_t dx = DBL_MAX;
      if (cluster == nullptr)
        dx = (posReference - posCandidate).Mag();
      else {
        auto hitsInCluster = cluster -> GetHitPtrs();
        for (auto hitIC : *hitsInCluster) {
          auto dx_RC = (hitIC->GetPosition() - posCandidate).Mag();
          if (dx_RC < dx)
            dx = dx_RC;
        }
      }
      if (dx > dx_cut)
        continue;



      // =======================================================================
      // TODO check residual of the hitCandidate to line fit of the cluster
      // =======================================================================

      if (cluster != nullptr) {
        auto residual = cluster -> Residual(posCandidate);
        if (residual > rs_cut)
          continue;
      }



      // =======================================================================
      // create cluster (if cluster does not exist)
      // =======================================================================

      if (cluster == nullptr)
      {
        cluster = (STNLHit *) fNLHitClusterArray -> ConstructedAt(countHitClusters);
        cluster -> SetClusterID(countHitClusters);
        ++countHitClusters;

        cluster -> AddHit(hitReference);
      }



      // =======================================================================
      // add hitCandidate to cluster and erase from the array
      // =======================================================================

      cluster -> AddHit(hitCandidate);
      hitArray.erase(hitArray.begin() + idxHit2);
    }

    if (cluster == nullptr)
      singleHitArray.push_back(hitReference);
  }



  // =======================================================================
  // compare cluster to left over hits only by the residual
  // =======================================================================

  Int_t numHitClusters = fNLHitClusterArray -> GetEntries();
  for (auto iCluster=0; iCluster<numHitClusters; ++iCluster)
  {
    if (singleHitArray.empty())
      break;

    auto cluster = (STNLHit *) fNLHitClusterArray -> At(iCluster);

    Int_t numCandidates = singleHitArray.size();
    for (auto iCandidate=numCandidates-1; iCandidate>=0; --iCandidate)
    {
      auto hitCandidate = singleHitArray.at(iCandidate);
      auto residual = cluster -> Residual(hitCandidate -> GetPosition());

      if (residual < rs_cut) {
        cluster -> AddHit(hitCandidate);
        hitArray.erase(singleHitArray.begin() + iCandidate);
      }
    }
  }

  // =======================================================================
  // compare cluster to left over hits only by the residual
  // =======================================================================

  Int_t numSingles = singleHitArray.size();
  for (auto hitSingle : singleHitArray)
  {
    auto cluster = (STNLHit *) fNLHitClusterArray -> ConstructedAt(countHitClusters);
    cluster -> SetClusterID(countHitClusters);
    ++countHitClusters;

    cluster -> AddHit(hitSingle);
  }

  LOG(INFO) << "  Event_" << fEventID << " : "
       << fBarArray -> GetEntries() << " bars,  "  
       << fNLHitArray -> GetEntries() << " hits,  " 
       << fNLHitClusterArray -> GetEntries() << " clusters." << FairLogger::endl;

  return;
}

void STNLDigiTask::SetBarPersistence(Bool_t value) { fIsBarPersistence = value; }
void STNLDigiTask::SetHitPersistence(Bool_t value) { fIsHitPersistence = value; }
void STNLDigiTask::SetHitClusterPersistence(Bool_t value) { fIsHitClusterPersistence = value; }

ClassImp(STNLDigiTask);
