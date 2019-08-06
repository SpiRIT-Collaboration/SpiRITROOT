#include "STNLDigiTask.hh"
#include "STDigiPar.hh"
#include "STNLHit.hh"
#include "STNeuLANDBar.hh"

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
#include "STMCPoint.hh"

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
  fGeoTrack = (TClonesArray*) ioman -> GetObject("GeoTracks");

  // output

  fMCPointArrayOut = new TClonesArray("STMCPoint");
  ioman -> Register("MCPoint","ST",fMCPointArrayOut,true);

  fGeoTrackOut = new TClonesArray("TGeoTrack");
  ioman -> Register("GeoTracks","ST",fGeoTrackOut,true);

  fBarArray = new TClonesArray("STNeuLANDBar");
  ioman -> Register("Bars","ST",fBarArray,true);

  fNLHitArray = new TClonesArray("STNLHit");
  ioman -> Register("NLHit","ST",fNLHitArray,true);

  fNLHitClusterArray = new TClonesArray("STNLHit");
  ioman -> Register("NLHitCluster","ST",fNLHitClusterArray,true);

  //

  fNL = STNeuLAND::GetNeuLAND();

  if (fCreateSummary)
  {
    TString filename = ioman -> GetOutFile() -> GetName();
    filename.ReplaceAll(".digi.",".summary.");

    fSummaryFile = new TFile(filename,"recreate");

    fSummaryTreeEvent = new TTree("data","");
    fSummaryTreeEvent -> Branch("qreco",&fSummaryQReco);
    fSummaryTreeEvent -> Branch("qmc",&fSummaryQMC);
    fSummaryTreeEvent -> Branch("nbars",&fSummaryNumBars);
    fSummaryTreeEvent -> Branch("nhits",&fSummaryNumHits);
    fSummaryTreeEvent -> Branch("nclusters",&fSummaryNumClusters);

    //fSummaryTreeHit = new TTree("hit","");
    //fSummaryTreeHit -> Branch("pos",&fSummaryPosHit);

    //fSummaryTreeMC = new TTree("mc","");
    //fSummaryTreeMC -> Branch("pos",&fSummaryPosMC);

    fSummaryTreeDist = new TTree("dist","");
    fSummaryTreeDist -> Branch("event",&fEventID);
    fSummaryTreeDist -> Branch("dist",&fSummaryTwoHitDist);
  }

  return kSUCCESS;
}

void 
STNLDigiTask::Exec(Option_t* option)
{
  ++fEventID;
  TString digiName = fName + Form("_%d",fEventID);

  fMCPointArrayOut -> Clear("C");
  fGeoTrackOut -> Clear("C");
  fBarArray -> Clear("C");
  fNLHitArray -> Clear("C");
  fNLHitClusterArray -> Clear("C");

  fSummaryIsGoodToFill = false;

  Int_t numGeoTracks = fGeoTrack -> GetEntries();
  for (auto iGeoTrack=0; iGeoTrack<numGeoTracks; ++iGeoTrack)
  {
    auto track1 = (TGeoTrack *) fGeoTrack    ->            At(iGeoTrack);
    auto track2 = (TGeoTrack *) fGeoTrackOut -> ConstructedAt(iGeoTrack);

    CopyGeoTrackFromTo(track1, track2);
  }

  Int_t numMCPoints = fMCPointArray -> GetEntries();

  if(numMCPoints<2) {
    LOG(INFO) << "  Event_" << fEventID << " : Not enough hits for digitization! ("<< numMCPoints << " < 2)" << FairLogger::endl;
    return;
  }

  STNeuLANDBar *bar = nullptr;
  map<Int_t, Int_t> mapBarIDToBarIdx;

  Int_t idxGoodMCPoint = -1;
  Int_t idxBar = -1;

  fSummaryQMC = 0;

  for(Int_t iMCPoint=0; iMCPoint<numMCPoints; iMCPoint++)
  {
    auto mcin = (STMCPoint*) fMCPointArray -> At(iMCPoint);

    //auto t = mcin -> GetTime();
    auto e = 1000 * mcin -> GetEnergyLoss(); // to MeV
    if (e <= 0)
      continue;

    auto trackID = mcin -> GetTrackID();

    TVector3 posg = 10 * TVector3(mcin -> GetX(), mcin -> GetY(), mcin -> GetZ());
    auto posl = fNL -> LocalPos(posg);

    auto d = fNL -> FindBarID(posl);
    if (d < 4000)
      continue;

    ++idxGoodMCPoint;

    auto mcout = (STMCPoint *) fMCPointArrayOut -> ConstructedAt(idxGoodMCPoint);
    mcout -> SetTrackID(mcin -> GetTrackID());
    mcout -> SetEnergyLoss(mcin -> GetEnergyLoss());
    mcout -> SetX(mcin -> GetX());
    mcout -> SetY(mcin -> GetY());
    mcout -> SetZ(mcin -> GetZ());

    fSummaryQMC += 1000 * mcout -> GetEnergyLoss();
    fSummaryPosMC = 10 * TVector3(mcout -> GetX(), mcout -> GetY(), mcout -> GetZ());
    if (fSummaryTreeMC != nullptr) fSummaryTreeMC -> Fill();

    if (mapBarIDToBarIdx.find(d) == mapBarIDToBarIdx.end()) {
      ++idxBar;
      bar = (STNeuLANDBar *) fBarArray -> ConstructedAt(idxBar);
      mapBarIDToBarIdx[d] = idxBar;

      bar -> SetBarID(d, fNL->GetLayer(d), fNL->GetRow(d),0,0);
      bar -> SetBar(fNL->IsAlongXNotY(d), fNL->GetBarLocalPosition(d));
    }
    else
      bar = (STNeuLANDBar *) fBarArray -> ConstructedAt(mapBarIDToBarIdx[d]);

    bar -> Fill(posl, e, idxGoodMCPoint);
  }

  //fBarArray -> Compress();

  // reconstruct neuland hits

  vector<STNLHit *> hitArray;

  fSummaryQReco = 0;

  TIter itBars(fBarArray);
  Int_t countHits = 0;
  STNLHit *hit = nullptr;
  while ((bar = (STNeuLANDBar *) itBars())) {

    if (bar -> FindHit(fThreshold))
    {
      hit = (STNLHit *) fNLHitArray -> ConstructedAt(countHits);
      auto position = bar -> GetTDCHitPosition();
      hit -> SetHitID(countHits);
      hit -> SetBarID(bar -> GetBarID());
      hit -> SetPosition(position);
      hit -> SetCharge(bar -> GetChargeA());

      hitArray.push_back(hit);

      fSummaryPosHit = hit -> GetPosition();
      if (fSummaryTreeHit != nullptr) fSummaryTreeHit -> Fill();

      ++countHits;
    }

    if (fCreateSummary) {
      fSummaryQReco += bar -> GetChargeA();

      fSummaryNumHits = fNLHitArray -> GetEntries();

      for (auto ihit=0; ihit<fSummaryNumHits; ++ihit)
      {
        auto hit1 = (STNLHit *) fNLHitArray -> At(ihit);
        for (auto jhit=0; jhit<fSummaryNumHits; ++jhit)
        {
          if (ihit == jhit)
            continue;
          auto hit2 = (STNLHit *) fNLHitArray -> At(jhit);
          fSummaryTwoHitDist = (hit1 -> GetPosition() - hit2 -> GetPosition()).Mag();
          fSummaryTreeDist -> Fill();
        }
      }
    }
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
        singleHitArray.erase(singleHitArray.begin() + iCandidate);
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

  fSummaryNumBars = idxBar+1;
  //fSummaryNumHits = fNLHitArray -> GetEntries();
  fSummaryNumClusters = fNLHitClusterArray -> GetEntries();
  fSummaryNumMCPoints = idxGoodMCPoint+1;
  fSummaryIsGoodToFill = true;

  LOG(INFO) << "  Event_" << fEventID << " : "
       << numGeoTracks << " geotrks,  "
       << idxGoodMCPoint+1 << " points,  "
       << idxBar+1 << " bars,  "
       << fNLHitArray -> GetEntries() << " hits,  " 
       << fNLHitClusterArray -> GetEntries() << " clusters." << FairLogger::endl;


  return;
}

void STNLDigiTask::FinishEvent()
{
  if (fCreateSummary && fSummaryIsGoodToFill) {
    if (fSummaryTreeEvent != nullptr) fSummaryTreeEvent -> Fill();
  }
}

void STNLDigiTask::FinishTask()
{
  if (fCreateSummary) {
    fSummaryFile -> cd();
    if (fSummaryTreeEvent != nullptr) fSummaryTreeEvent -> Write();
    if (fSummaryTreeHit   != nullptr) fSummaryTreeHit   -> Write();
    if (fSummaryTreeMC    != nullptr) fSummaryTreeMC    -> Write();
    if (fSummaryTreeDist  != nullptr) fSummaryTreeDist  -> Write();

    LOG(INFO) << "Summary file " << fSummaryFile -> GetName() << "create." << FairLogger::endl;
    fSummaryFile -> ls();
    fSummaryFile -> Close();
  }
}

void STNLDigiTask::SetBarPersistence(Bool_t value) { fIsBarPersistence = value; }
void STNLDigiTask::SetHitPersistence(Bool_t value) { fIsHitPersistence = value; }
void STNLDigiTask::SetHitClusterPersistence(Bool_t value) { fIsHitClusterPersistence = value; }

void STNLDigiTask::CopyGeoTrackFromTo(TGeoTrack *from, TGeoTrack *to)
{
  to -> ResetTrack();

  to -> SetId(from -> GetId());
  to -> SetPDG(from -> GetPDG());

  auto parent = from -> GetMother();
  if (parent != nullptr)
    to -> SetParent(parent);

  auto particle = from -> GetParticle();
  if (particle != nullptr)
    to -> SetParticle(particle);

  auto numTracks = from -> GetNdaughters();
  for (auto iTrack=0; iTrack<numTracks; ++iTrack)
  {
    auto daughter = from -> GetDaughter(iTrack);
    if (daughter == nullptr)
      break;
    to -> AddDaughter(daughter);
  }

  auto numPoints = from -> GetNpoints();
  for (auto iGeoPoint=0; iGeoPoint<numPoints; ++iGeoPoint)
  {
    Double_t x, y, z, t;
    from -> GetPoint(iGeoPoint, x, y, z, t);
    to -> AddPoint(x, y, z, t);
  }
}

void STNLDigiTask::SetThreshold(Double_t threshold)
{
  fThreshold = threshold;
  LOG(INFO) << "NeuLAND bar threshold is set to " << fThreshold << FairLogger::endl;
}

ClassImp(STNLDigiTask);
