#include "STHelixTrackFinder.hh"

#include <iostream>
using namespace std;

#include "STGlobal.hh"

ClassImp(STHelixTrackFinder)

STHelixTrackFinder::STHelixTrackFinder()
{
  fFitter   = new STHelixTrackFitter();
  fEventMap = new STPadPlaneMap();
  fHitMap   = new STPadPlaneMap();

  fCandHits = new std::vector<STHit*>;
  fGoodHits = new std::vector<STHit*>;
  fBadHits  = new std::vector<STHit*>;
}

void 
STHelixTrackFinder::BuildTracks(TClonesArray *hitArray, TClonesArray *trackArray, TClonesArray *hitClusterArray)
{
  fTrackArray      = trackArray;
  fHitClusterArray = hitClusterArray;
  fEventMap -> Clear();
  fCandHits -> clear();
  fGoodHits -> clear();
  fBadHits  -> clear();
  fHitMap   -> Clear();

  Int_t numTotalHits = hitArray -> GetEntries();
  for (Int_t iHit = 0; iHit < numTotalHits; iHit++) {
    auto hit = (STHit *) hitArray -> At(iHit);
    if (fCRadius != -1 && fZLength != -1) {
      auto hitPos = hit -> GetPosition();
      auto r = (fCutCenter - hitPos).Perp();
      auto z = hitPos.Z();
      if (r <= fCRadius && z <= fZLength)
        continue;
    } else if (fSRadius != -1) {
      auto hitPos = hit -> GetPosition();
      auto r = (fCutCenter - hitPos).Mag();
      if (r <= fCRadius)
        continue;
    } else if (fERadii != TVector3(-1, -1, -1)) {
      auto hitPos = hit -> GetPosition();
      auto rVec = (fCutCenter - hitPos);
      rVec = TVector3(rVec.X()/fERadii.X(), rVec.Y()/fERadii.Y(), rVec.Z()/fERadii.Z());
      if (rVec.Mag() <= 1)
        continue;
    }

    fEventMap -> AddHit((STHit *) hitArray -> At(iHit));
    fHitMap -> AddHit((STHit *) hitArray -> At(iHit));
  }

  while(1)
  {
    fCandHits -> clear();
    fGoodHits -> clear();
    fBadHits  -> clear();

    STHelixTrack *track = NewTrack();
    if (track == nullptr)
      break;

    bool survive = TrackInitialization(track);
    survive = true;

    while (survive) {
      TrackContinuum(track);
      survive = TrackQualityCheck(track);
      if (!survive)
        break;

      TrackExtrapolation(track);
      survive = TrackQualityCheck(track);
      if (!survive)
        break;

      TrackConfirmation(track);
      break;
    }

    for (auto badHit : *fBadHits)
      fEventMap -> AddHit(badHit);
    fBadHits -> clear();

    if (track -> TrackLength() < 150 || track -> GetHelixRadius() < 25)
      survive = false;

    if (survive) {
      auto trackHits = track -> GetHitArray();
      auto trackID = track -> GetTrackID();
      for (auto trackHit : *trackHits) {
        trackHit -> AddTrackCand(trackID);
        fEventMap -> AddHit(trackHit);
      }
    }
    else {
      auto trackHits = track -> GetHitArray();
      for (auto trackHit : *trackHits) {
        trackHit -> AddTrackCand(-1);
        fEventMap -> AddHit(trackHit);
      }
      fTrackArray -> Remove(track);
    }
  }
  fTrackArray -> Compress();

  TVector3 vertex = FindVertex(fTrackArray);

  auto numTracks = fTrackArray -> GetEntries();
  for (auto iTrack = 0; iTrack < numTracks; iTrack++) {
    auto track = (STHelixTrack *) fTrackArray -> At(iTrack);
    track -> DetermineParticleCharge(vertex);

    track -> FinalizeHits();
    //    HitClustering(track);
       HitClusteringMar4(track);

    track -> FinalizeClusters();
  }
}

STHelixTrack *
STHelixTrackFinder::NewTrack()
{
  STHit *hit = fEventMap -> PullOutNextFreeHit();
  if (hit == nullptr)
    return nullptr;

  Int_t idx = fTrackArray -> GetEntries();
  STHelixTrack *track = new ((*fTrackArray)[idx]) STHelixTrack(idx);
  track -> AddHit(hit);
  fGoodHits -> push_back(hit);

  return track;
}

STHitCluster *
STHelixTrackFinder::NewCluster(STHit *hit)
{
  Int_t idx = fHitClusterArray -> GetEntries();
  STHitCluster *cluster = new ((*fHitClusterArray)[idx]) STHitCluster();
  cluster -> AddHit(hit);
  cluster -> SetClusterID(idx);
  return cluster;
}

bool
STHelixTrackFinder::TrackInitialization(STHelixTrack *track)
{
  fEventMap -> PullOutNeighborHits(fGoodHits, fCandHits);
  fGoodHits -> clear();

  Int_t numCandHits = fCandHits -> size();;

  while (numCandHits != 0) {
    sort(fCandHits->begin(), fCandHits->end(), STHitByDistanceTo(track->GetMean()));

    for (Int_t iHit = 0; iHit < numCandHits; iHit++) {
      STHit *candHit = fCandHits -> back();
      fCandHits -> pop_back();

      Double_t quality = CorrelateSimple(track, candHit);

      if (quality > 0) {
        fGoodHits -> push_back(candHit);
        track -> AddHit(candHit);

        if (track -> GetNumHits() > 6) {
          if (track -> GetNumHits() > 15) {
            for (auto candHit2 : *fCandHits)
              fEventMap -> AddHit(candHit2);
            fCandHits -> clear();
            break;
          }

          fFitter -> Fit(track);

          if (!(track -> GetNumHits() < 10 && track -> GetHelixRadius() < 30) && (track -> TrackLength() > fDefaultScale * track -> GetRMSW()))
            return true;
        }
        fFitter -> FitPlane(track);
      }
      else
        fBadHits -> push_back(candHit);
    }

    fEventMap -> PullOutNeighborHits(fGoodHits, fCandHits);
    fGoodHits -> clear();

    numCandHits = fCandHits -> size();
  }

  for (auto badHit : *fBadHits)
    fEventMap -> AddHit(badHit);
  fBadHits -> clear();

  return false;
}

bool 
STHelixTrackFinder::TrackContinuum(STHelixTrack *track)
{
  fEventMap -> PullOutNeighborHits(fGoodHits, fCandHits);
  fGoodHits -> clear();

  Int_t numCandHits = fCandHits -> size();

  while (numCandHits != 0)
  {
    sort(fCandHits -> begin(), fCandHits -> end(), STHitSortCharge());

    for (Int_t iHit = 0; iHit < numCandHits; iHit++) {
      STHit *candHit = fCandHits -> back();
      fCandHits -> pop_back();

      Double_t quality = 0; 
      if (CheckHitOwner(candHit) == -2)
        quality = Correlate(track, candHit);

      if (quality > 0) {
        fGoodHits -> push_back(candHit);
        track -> AddHit(candHit);
        fFitter -> Fit(track);
      } else
        fBadHits -> push_back(candHit);
    }

    fEventMap -> PullOutNeighborHits(fGoodHits, fCandHits);
    fGoodHits -> clear();

    numCandHits = fCandHits -> size();
  }

  return true;
}

bool 
STHelixTrackFinder::TrackExtrapolation(STHelixTrack *track)
{
  for (auto badHit : *fBadHits)
    fEventMap -> AddHit(badHit);
  fBadHits -> clear();

  Int_t count = 0;
  bool buildHead = true;
  Double_t extrapolationLength = 0;
  while (AutoBuildByExtrapolation(track, buildHead, extrapolationLength)) {
    if (count++ > 100)
      break;
  }

  count = 0;
  buildHead = !buildHead;
  extrapolationLength = 0;
  while (AutoBuildByExtrapolation(track, buildHead, extrapolationLength)) {
    if (count++ > 100)
      break;
  }

  for (auto badHit : *fBadHits)
    fEventMap -> AddHit(badHit);
  fBadHits -> clear();

  return true;
}

bool 
STHelixTrackFinder::AutoBuildByExtrapolation(STHelixTrack *track, bool &buildHead, Double_t &extrapolationLength)
{
  auto helicity = track -> Helicity();

  TVector3 p;
  if (buildHead) p = track -> ExtrapolateHead(extrapolationLength);
  else           p = track -> ExtrapolateTail(extrapolationLength);

  return AutoBuildAtPosition(track, p, buildHead, extrapolationLength);
}

bool 
STHelixTrackFinder::AutoBuildByInterpolation(STHelixTrack *track, bool &tailToHead, Double_t &extrapolationLength, Double_t rScale)
{
  TVector3 p;
  if (tailToHead) p = track -> InterpolateByLength(extrapolationLength);
  else            p = track -> InterpolateByLength(track -> TrackLength() - extrapolationLength);

  return AutoBuildAtPosition(track, p, tailToHead, extrapolationLength, rScale);
}

bool 
STHelixTrackFinder::AutoBuildAtPosition(STHelixTrack *track, TVector3 p, bool &tailToHead, Double_t &extrapolationLength, Double_t rScale)
{
  if (p.X() < -432 || p.X() < -432 || p.Z() < 0 || p.Z() > 1344 || p.Y() < -530 || p.Y() > 0)
    return false;

  auto helicity = track -> Helicity();

  Double_t rms = 3*track -> GetRMSW();
  if (rms < 25) 
    rms = 25;

  fEventMap -> PullOutNeighborHits(p, rms, fCandHits);
  sort(fCandHits -> begin(), fCandHits -> end(), STHitSortCharge());

  Int_t numCandHits = fCandHits -> size();
  Bool_t foundHit = false;

  for (Int_t iHit = 0; iHit < numCandHits; iHit++) {
    STHit *candHit = fCandHits -> back();
    fCandHits -> pop_back();

    Double_t quality = 0; 
    if (CheckHitOwner(candHit) < 0) 
      quality = Correlate(track, candHit, rScale);

    if (quality > 0) {
      track -> AddHit(candHit);
      fFitter -> Fit(track);
      foundHit = true;
    } else
      fBadHits -> push_back(candHit);
  }

  if (foundHit) {
    extrapolationLength = 10; 
    if (helicity != track -> Helicity())
      tailToHead = !tailToHead;
  }
  else {
    extrapolationLength += 10; 
    if (extrapolationLength > 0.8 * track -> TrackLength()) {
      return false;
    }
  }

  return true;
}

bool
STHelixTrackFinder::TrackQualityCheck(STHelixTrack *track)
{
  Double_t continuity = track -> Continuity();
  if (continuity < .6) {
    if (track -> TrackLength() * continuity < 500)
      return false;
  }

  if (track -> GetHelixRadius() < 25)
    return false;

  return true;
}

bool
STHelixTrackFinder::TrackConfirmation(STHelixTrack *track)
{
  auto tailToHead = false;
  if (track -> PositionAtTail().Z() > track -> PositionAtHead().Z())
    tailToHead = true;

  for (auto badHit : *fBadHits)
    fEventMap -> AddHit(badHit);
  fBadHits -> clear();
  ConfirmHits(track, tailToHead);

  tailToHead = !tailToHead; 

  for (auto badHit : *fBadHits)
    fEventMap -> AddHit(badHit);
  fBadHits -> clear();
  ConfirmHits(track, tailToHead);

  for (auto badHit : *fBadHits)
    fEventMap -> AddHit(badHit);
  fBadHits -> clear();

  return true;
}


bool
STHelixTrackFinder::ConfirmHits(STHelixTrack *track, bool &tailToHead)
{
  track -> SortHits(!tailToHead);
  auto trackHits = track -> GetHitArray();
  auto numHits = trackHits -> size();

  TVector3 q, m;
  auto lPre = track -> ExtrapolateByMap(trackHits->at(numHits-1)->GetPosition(), q, m);

  auto extrapolationLength = 10.;
  for (auto iHit = 1; iHit < numHits; iHit++) 
  {
    STHit *trackHit = trackHits -> at(numHits-iHit-1);
    auto lCur = track -> ExtrapolateByMap(trackHit->GetPosition(), q, m);

    Double_t quality = Correlate(track, trackHit);

    if (quality <= 0) {
      track -> Remove(trackHit);
      trackHit -> RemoveTrackCand(trackHit -> GetTrackID());
      auto helicity = track -> Helicity();
      fFitter -> Fit(track);
      if (helicity != track -> Helicity())
        tailToHead = !tailToHead;
    }

    auto dLength = abs(lCur - lPre);
    extrapolationLength = 10;
    while(dLength > 0 && AutoBuildByInterpolation(track, tailToHead, extrapolationLength, 1)) { dLength -= 10; }
  }

  extrapolationLength = 0;
  while (AutoBuildByExtrapolation(track, tailToHead, extrapolationLength)) {}

  return true;
}

bool
STHelixTrackFinder::HitClustering(STHelixTrack *helix)
{
  auto hitArray = helix -> GetHitArray();
  
  TObjArray hits;
  for (auto hit : *hitArray)
    hits.Add(hit);

  Int_t numClusters = fHitClusterArray -> GetEntries();
  Int_t idxCluster = fHitClusterArray -> GetEntries();

  TVector3 q;
  Double_t alpha;

  bool isRow = 0;
  auto section = 0;
  bool currentBuildIsRow = 1; // should be different from isRow so it can init as son as loop start.
  auto currentBuildSection = -999; // should be different from section so it can init as son as loop start.

  TObjArray buildingClusters[2];
  Int_t buildingSection[2] = {-1,-1}; // 0: layer, 1: row
  Int_t numBuildingClusters[2] = {0}; // 0: layer, 1: row
  Int_t buildBoundary[2][2];
  buildBoundary[0][0] =  999;
  buildBoundary[0][1] = -999;
  buildBoundary[1][0] =  999;
  buildBoundary[1][1] = -999;

  hits.Sort();
  TIter itHits(&hits);
  STHit *hit;

  vector<Int_t> sectionHistory;

  while ((hit = (STHit *) itHits.Next()))
  {
    helix -> ExtrapolateToPointAlpha(hit -> GetPosition(), q, alpha);

    double alpha_a = abs(alpha);
    double pi = TMath::Pi();
    double beta = 45 * TMath::DegToRad(); //enter angle for which a track switches layer to row clust

    //There are four quadrants or sections starting from section 0 its layer clustering
    //section 1 row, section 2 layer, section 3 row
    if( alpha_a > beta && alpha_a <= (pi-beta))
      {
	isRow = 1;
	section = 1;
      }
    else if( alpha_a > (pi-beta) && alpha_a <= (pi+beta))
      {
	isRow = 0;
	section = 2;
      }
    else if(alpha_a > (pi+beta) && alpha_a <= (2*pi - beta))
      {
	isRow = 1;
	section = 3;
      }
    else
      {
	isRow = 0;
	section = 0;
      }

    if (section != currentBuildSection) // init
    {
      for (auto section0 : sectionHistory)
        if (section0 == section)
          continue;

      sectionHistory.push_back(section);

      currentBuildIsRow = isRow;
      currentBuildSection = section;
      vector<Int_t> initList; // 1 for row, 0 for layer

      if (currentBuildSection-section > 1) // if jumped section
      {
        if (buildingSection[1] > buildingSection[0]) {
          initList.push_back(0);
          initList.push_back(1);
        } else {
          initList.push_back(1);
          initList.push_back(0);
        }
      }
      else if (isRow) initList.push_back(1);
      else            initList.push_back(0);

      for (auto rl : initList) {
        buildingClusters[rl].Clear();
        numBuildingClusters[rl] = 0;
        buildBoundary[abs(rl-1)][0] = 999;
        buildBoundary[abs(rl-1)][1] = -999;
      }

      //TODO
      continue;
    }
    else // continue build with same option
    {
      if (isRow) {
        auto row = hit -> GetRow();
        auto layer = hit -> GetLayer();

        //shoud check opposite boundary because next build will use shade area
        if (layer < buildBoundary[0][0]) buildBoundary[0][0] = layer;
        if (layer > buildBoundary[0][1]) buildBoundary[0][1] = layer;

        // check before build
        bool foundCluster = false;
        for (auto iCluster = numBuildingClusters[0]-1; iCluster >= 0; --iCluster) {
          auto cluster = (STHitCluster *) buildingClusters[0].At(iCluster);
          if (cluster -> GetLayer() == layer) {
            cluster -> SetClusterID(-1); // bad cluster
            foundCluster = true;
            break;
          }
        }
        if(foundCluster)
          continue;

        // check this build
        if (row >= buildBoundary[1][0] && row <= buildBoundary[1][1])
          continue;

        foundCluster = false;
        for (auto iCluster = numBuildingClusters[1]-1; iCluster >= 0; --iCluster) {
          auto cluster = (STHitCluster *) buildingClusters[1].At(iCluster);
          if (cluster -> GetRow() == row) {
            foundCluster = true;
            cluster -> AddHit(hit);
            break;
          }
        }
        if (!foundCluster) {
          auto cluster = new ((*fHitClusterArray)[numClusters++]) STHitCluster();
          cluster -> SetClusterID(1);
          cluster -> SetRow(row);
          cluster -> SetLayer(-1);
          cluster -> AddHit(hit);
          buildingClusters[1].Add(cluster);
          ++numBuildingClusters[1];
        }
      } ////////////////////////////////////////////////////// was for row build
      else {
        auto row = hit -> GetRow();
        auto layer = hit -> GetLayer();

        //shoud check opposite boundary because next build will use shade area
        if (row < buildBoundary[1][0]) buildBoundary[1][0] = row;
        if (row > buildBoundary[1][1]) buildBoundary[1][1] = row;

        // check before build
        bool foundCluster = false;
        for (auto iCluster = numBuildingClusters[1]-1; iCluster >= 0; --iCluster) {
          auto cluster = (STHitCluster *) buildingClusters[1].At(iCluster);
          if (cluster -> GetRow() == row) {
            cluster -> SetClusterID(-1); // bad cluster
            foundCluster = true;
            break;
          }
        }
        if(foundCluster)
          continue;

        // check this build
        if (layer >= buildBoundary[0][0] && layer <= buildBoundary[0][1])
          continue;

        foundCluster = false;
        for (auto iCluster = numBuildingClusters[0]-1; iCluster >= 0; --iCluster) {
          auto cluster = (STHitCluster *) buildingClusters[0].At(iCluster);
          if (cluster -> GetLayer() == layer) {
            foundCluster = true;
            cluster -> AddHit(hit);
            break;
          }
        }
        if (!foundCluster) {
          auto cluster = new ((*fHitClusterArray)[numClusters++]) STHitCluster();
          cluster -> SetClusterID(1);
          cluster -> SetRow(-1);
          cluster -> SetLayer(layer);
          cluster -> AddHit(hit);
          buildingClusters[0].Add(cluster);
          ++numBuildingClusters[0];
        }
      }
    } ////////////////////////////////////////////////////// was for layer build
  }

  for (auto iCluster = idxCluster; iCluster < numClusters; ++iCluster) {
    auto cluster = (STHitCluster *) fHitClusterArray -> At(iCluster);

    auto clusterPos = cluster -> GetPosition();
    auto x = clusterPos.X();
    auto y = clusterPos.Y();
    auto z = clusterPos.Z();

    auto rVec = clusterPos - fCutCenter;
    auto rPerp = rVec.Perp();
    auto rMag = rVec.Mag();
    rVec = TVector3(rVec.X()/(fERadii.X() + fCutMargin), rVec.Y()/(fERadii.Y() + fCutMargin), rVec.Z()/(fERadii.Z() + fCutMargin));

    if (cluster -> GetClusterID() == -1)
      fHitClusterArray -> Remove(cluster);
    else if (cluster -> IsLayerCluster() && cluster -> GetDx() < 1.e-2)
      fHitClusterArray -> Remove(cluster);
    else if (cluster -> IsRowCluster() && cluster -> GetDz() < 1.e-2)
      fHitClusterArray -> Remove(cluster);
    else if (x >= fCCLeft || x <= fCCRight || y >= fCCTop || y <= fCCBottom)
      fHitClusterArray -> Remove(cluster);
    else if (fZLength != -1 && fCRadius != -1 && z <= fZLength + fCutMargin && rPerp <= fCRadius + fCutMargin)
      fHitClusterArray -> Remove(cluster);
    else if (fSRadius != -1 && rMag <= fSRadius + fCutMargin)
      fHitClusterArray -> Remove(cluster);
    else if (fERadii != TVector3(-1, -1, -1) && rVec.Mag() <= 1)
      fHitClusterArray -> Remove(cluster);
    else {
      cluster -> SetClusterID(idxCluster++);
      helix -> AddHitCluster(cluster);
    }
  }

  fHitClusterArray -> Compress();

  //Store hitIDs of hits in helix in a mapped array for quick searching
  //We will pass to neighbor checking funciton to make sure neighbors are not hits in helix track
  auto hit_IDary = helix -> GetHitIDArray();
  std::vector<bool> helix_hits(*max_element(hit_IDary->begin(), hit_IDary->end())+1,false);
      for( auto hitID : *hit_IDary)
	  helix_hits.at(hitID)=true;

  //Here we count how many hits in the cluster are neighbors to dead pads by saturation
  auto helix_cl_ary = helix -> GetClusterArray();
   for (auto iCluster = 0; iCluster < helix_cl_ary -> size(); ++iCluster) 
    {
      auto cluster = (STHitCluster *) helix_cl_ary -> at(iCluster);
      auto hit_ary = cluster -> GetHitPtrs();
      bool by_row = cluster -> IsRowCluster();
      int sat_hits = 0;
      for( auto cl_hit : *hit_ary)
	{
	  //	  if( cl_hit -> IsSaturated() )//the saturated hits cannot shadow themselves 
	  //	    continue;
	  if( fHitMap -> IsNeighborSaturated(cl_hit,by_row,helix_hits) ) //has a saturated neighbor 
	    sat_hits++;
	}
      cluster -> SetNumSatNeighbors(sat_hits);
      if(sat_hits != 0)
	{
	  cluster -> SetIsMissingCharge(true); //missing charge due to saturated neighbor killed pad 
	  cluster -> SetFractSatNeighbors(sat_hits/hit_ary->size());
	}

    }

  return true;
}

bool
STHelixTrackFinder::HitClusteringMar4(STHelixTrack *helix)
{
  helix -> SortHitsByTimeOrder();

  auto helixHits = helix -> GetHitArray();
  auto numHits = helixHits -> size();

  auto SetClusterLength = [helix](STHitCluster *cluster) {
    auto row = cluster -> GetRow();
    auto layer = cluster -> GetLayer();
    auto alpha = helix -> AlphaAtPosition(cluster -> GetPosition());
    Double_t length;
    TVector3 q0;
    TVector3 q1;
    if (layer == -1) {
      Double_t x0 = (row)*8.-432.;
      Double_t x1 = (row+1)*8.-432.;
      length = 1;
      helix -> ExtrapolateToX(x0, alpha, q0);
      helix -> ExtrapolateToX(x1, alpha, q1);
      length = TMath::Abs(helix -> Map(q0).Z() - helix -> Map(q1).Z());
    } else {
      Double_t z0 = (layer)*12.;
      Double_t z1 = (layer+1)*12.;
      helix -> ExtrapolateToZ(z0, alpha, q0);
      helix -> ExtrapolateToZ(z1, alpha, q1);
      length = TMath::Abs(helix -> Map(q0).Z() - helix -> Map(q1).Z());
    }
    cluster -> SetLength(length);
  };

  bool buildNewCluster = true;
  auto curHit = helixHits -> at(0);
  bool buildByLayer = CheckBuildByLayer(helix, curHit, nullptr);

  STHitCluster *lastCluster = nullptr;
  lastCluster = NewCluster(curHit);

  Int_t rowMin = curHit -> GetRow();
  Int_t rowMax = curHit -> GetRow();
  Int_t layerMin = curHit -> GetLayer();
  Int_t layerMax = curHit -> GetLayer();

  if (buildByLayer) {
    layerMin = layerMin = 0;
    layerMin = layerMax = -1;
    lastCluster -> SetRow(-1);
    lastCluster -> SetLayer(curHit -> GetLayer());
  } else {
    rowMin = rowMin = 0;
    rowMin = rowMax = -1;
    lastCluster -> SetRow(curHit -> GetRow());
    lastCluster -> SetLayer(-1);
  }
  SetClusterLength(lastCluster);

  std::vector<STHitCluster *> buildingClusters;
  buildingClusters.push_back(lastCluster);

  for (auto iHit = 1; iHit < numHits; iHit++)
  {
    curHit = helixHits -> at(iHit);

    auto row = curHit -> GetRow();
    auto layer = curHit -> GetLayer();

    if (!buildNewCluster)
    {
      if (buildByLayer) {
        if (row < rowMin || row > rowMax) {
          buildNewCluster = true;
          layerMin = layer;
          layerMax = layer;
        }
      } else {
        if (layer < layerMin || layer > layerMax) {
          buildNewCluster = true;
          rowMin = row;
          rowMax = row;
        }
      }

      if (buildNewCluster) {
        buildingClusters.clear();
        buildByLayer = !buildByLayer;
      }
    }

    if (buildByLayer)
    {
      bool createNewCluster = true;
      if (layer < layerMin || layer > layerMax) {
        for (auto cluster : buildingClusters) {
          if (layer == cluster -> GetLayer()) {
            createNewCluster = false;
            cluster -> AddHit(curHit);
          }
        }
      } else
        createNewCluster = false;

      if (buildNewCluster) {
        if (row < rowMin) rowMin = row;
        if (row > rowMax) rowMax = row;
      } else
        createNewCluster = false;

      if (createNewCluster) {
        if (buildByLayer !=  CheckBuildByLayer(helix, curHit, (fClusteringMargin > 0 ? helixHits -> at(iHit - 1) : nullptr))) {
          buildNewCluster = false;
          lastCluster = nullptr;
        }
        else {
          if (lastCluster != nullptr) {
            helix -> AddHitCluster(lastCluster);
            lastCluster -> SetIsStable(true);
          }
          lastCluster = NewCluster(curHit);
          lastCluster -> SetRow(-1);
          lastCluster -> SetLayer(layer);
          SetClusterLength(lastCluster);
          buildingClusters.push_back(lastCluster);
        }
      }
    }
    else
    {
      bool createNewCluster = true;
      if (row < rowMin || row > rowMax) {
        for (auto cluster : buildingClusters) {
          if (row == cluster -> GetRow()) {
            createNewCluster = false;
            cluster -> AddHit(curHit);
          }
        }
      } else
        createNewCluster = false;

      if (buildNewCluster) {
        if (layer < layerMin) layerMin = layer;
        if (layer > layerMax) layerMax = layer;
      } else
        createNewCluster = false;

      if (createNewCluster) {
        if (buildByLayer !=  CheckBuildByLayer(helix, curHit, (fClusteringMargin > 0 ? helixHits -> at(iHit - 1) : nullptr))) {
          buildNewCluster = false;
          lastCluster = nullptr;
        }
        else {
          if (lastCluster != nullptr) {
            helix -> AddHitCluster(lastCluster);
            lastCluster -> SetIsStable(true);
          }
          lastCluster = NewCluster(curHit);
          lastCluster -> SetRow(row);
          lastCluster -> SetLayer(-1);
          SetClusterLength(lastCluster);
          buildingClusters.push_back(lastCluster);
        }
      }
    }
  }

  Int_t numCluster = fHitClusterArray -> GetEntries();
  for (auto iCluster = 0; iCluster < numCluster; iCluster++)
  {
    auto cluster = (STHitCluster *) fHitClusterArray -> At(iCluster);

    if (cluster -> IsStable()) {
      auto clusterPos = cluster -> GetPosition();
      auto x = clusterPos.X();
      auto y = clusterPos.Y();
      auto z = clusterPos.Z();

      auto rVec = clusterPos - fCutCenter;
      auto rPerp = rVec.Perp();
      auto rMag = rVec.Mag();
      rVec = TVector3(rVec.X()/(fERadii.X() + fCutMargin), rVec.Y()/(fERadii.Y() + fCutMargin), rVec.Z()/(fERadii.Z() + fCutMargin));

      if (z < 20)
        cluster -> SetIsStable(false);
      else if (x >= fCCLeft || x <= fCCRight || y >= fCCTop || y <= fCCBottom)
        cluster -> SetIsStable(false);
      else if (fZLength != -1 && fCRadius != -1 && z <= fZLength + fCutMargin && rPerp <= fCRadius + fCutMargin)
        cluster -> SetIsStable(false);
      else if (fSRadius != -1 && rMag <= fSRadius + fCutMargin)
        cluster -> SetIsStable(false);
      else if (fERadii != TVector3(-1, -1, -1) && rVec.Mag() <= 1)
        cluster -> SetIsStable(false);
      else
        CheckIsContinuousHits(cluster);
    }
  }

  if (helix -> GetNumClusters() < 5) {
    helix -> SetIsBad();
    return false;
  }

  if (fFitter -> FitCluster(helix) == false) {
    fFitter -> Fit(helix);
    helix -> SetIsLine();
  }

  //Store hitIDs of hits in helix in a mapped array for quick searching
  //We will pass to neighbor checking funciton to make sure neighbors are not hits in helix track
  auto hit_IDary = helix -> GetHitIDArray();
  std::vector<bool> helix_hits(*max_element(hit_IDary->begin(), hit_IDary->end())+1,false);
      for( auto hitID : *hit_IDary)
	  helix_hits.at(hitID)=true;

  //Here we count how many hits in the cluster are neighbors to dead pads by saturation
  auto helix_cl_ary = helix -> GetClusterArray();
   for (auto iCluster = 0; iCluster < helix_cl_ary -> size(); ++iCluster) 
    {
      auto cluster = (STHitCluster *) helix_cl_ary -> at(iCluster);
      auto hit_ary = cluster -> GetHitPtrs();
      bool by_row = cluster -> IsRowCluster();
      int sat_hits = 0;
      for( auto cl_hit : *hit_ary)
	{
	  //	  if( cl_hit -> IsSaturated() )//the saturated hits cannot shadow themselves 
	  //	    continue;
	  if( fHitMap -> IsNeighborSaturated(cl_hit,by_row,helix_hits) ) //has a saturated neighbor 
	    sat_hits++;
	}
      cluster -> SetNumSatNeighbors(sat_hits);
      if(sat_hits != 0)
	{
	  cluster -> SetIsMissingCharge(true); //missing charge due to saturated neighbor killed pad 
	  cluster -> SetFractSatNeighbors(sat_hits/hit_ary->size());
	}

    }

  return true;
}

Int_t
STHelixTrackFinder::CheckHitOwner(STHit *hit)
{
  auto candTracks = hit -> GetTrackCandArray();
  if (candTracks -> size() == 0)
    return -2;

  Int_t trackID = -1;
  for (auto candTrackID : *candTracks) {
    if (candTrackID != -1) {
      trackID = candTrackID;
    }
  }

  return trackID;
}

Double_t 
STHelixTrackFinder::Correlate(STHelixTrack *track, STHit *hit, Double_t rScale)
{
  Double_t scale = rScale * fDefaultScale;
  Double_t trackLength = track -> TrackLength();
  if (trackLength < 500.)
    scale = scale + (500. - trackLength)/500.;

  Double_t rmsWCut = track -> GetRMSW();
  if (rmsWCut < fTrackWCutLL) rmsWCut = fTrackWCutLL;
  if (rmsWCut > fTrackWCutHL) rmsWCut = fTrackWCutHL;
  rmsWCut = scale * rmsWCut;

  Double_t rmsHCut = track -> GetRMSH();
  if (rmsHCut < fTrackHCutLL) rmsHCut = fTrackHCutLL;
  if (rmsHCut > fTrackHCutHL) rmsHCut = fTrackHCutHL;
  rmsHCut = scale * rmsHCut;

  auto qHead = track -> Map(track -> PositionAtHead());
  auto qTail = track -> Map(track -> PositionAtTail());
  auto q = track -> Map(hit -> GetPosition());

  auto LengthAlphaCut = [track](Double_t dLength) {
    if (dLength > 0) {
      if (dLength > .5*track -> TrackLength()) {
        if (abs(track -> AlphaByLength(dLength)) > .5*TMath::Pi()) {
          return true;
        }
      }
    }
    return false;
  };

  if (qHead.Z() > qTail.Z()) {
    if (LengthAlphaCut(q.Z() - qHead.Z())) return 0;
    if (LengthAlphaCut(qTail.Z() - q.Z())) return 0;
  } else {
    if (LengthAlphaCut(q.Z() - qTail.Z())) return 0;
    if (LengthAlphaCut(qHead.Z() - q.Z())) return 0;
  }

  Double_t dr = abs(q.X());
  Double_t quality = 0;
  if (dr < rmsWCut && abs(q.Y()) < rmsHCut)
    quality = sqrt((dr-rmsWCut)*(dr-rmsWCut)) / rmsWCut;

  return quality;
}

Double_t 
STHelixTrackFinder::CorrelateSimple(STHelixTrack *track, STHit *hit)
{
  if (hit -> GetNumTrackCands() != 0)
    return 0;

  Double_t quality = 0;

  Int_t row = hit -> GetRow();
  Int_t layer = hit -> GetLayer();

  auto trackHits = track -> GetHitArray();
  bool ycut = false;
  for (auto trackHit : *trackHits) {
    if (row == trackHit -> GetRow() && layer == trackHit -> GetLayer())
      return 0;
    if (abs(hit->GetY() - trackHit->GetY()) < 12)
      ycut = true;
  }
  if (ycut == false)
    return 0;

  if (track -> IsBad()) {
    quality = 1;
  }
  else if (track -> IsLine()) {
    auto perp = track -> PerpLine(hit -> GetPosition());

    Double_t rmsCut = track -> GetRMSH();
    if (rmsCut < fTrackHCutLL) rmsCut = fTrackHCutLL;
    if (rmsCut > fTrackHCutHL) rmsCut = fTrackHCutHL;
    rmsCut = 3 * rmsCut;

    if (perp.Y() > rmsCut)
      quality = 0;
    else {
      perp.SetY(0);
      if (perp.Mag() < 15)
        quality = 1;
    }
  }
  else if (track -> IsPlane()) {
    Double_t dist = (track -> PerpPlane(hit -> GetPosition())).Mag();

    Double_t rmsCut = track -> GetRMSH();
    if (rmsCut < fTrackHCutLL) rmsCut = fTrackHCutLL;
    if (rmsCut > fTrackHCutHL) rmsCut = fTrackHCutHL;
    rmsCut = 3 * rmsCut;

    if (dist < rmsCut)
      quality = 1;
  }

  return quality;
}

Double_t 
STHelixTrackFinder::TangentOfMaxDipAngle(STHit *hit)
{
  TVector3 v(0, -213.3, -89);
  TVector3 p = hit -> GetPosition();

  Double_t dx = p.X()-v.X();
  Double_t dy = p.Y()-v.Y();
  Double_t dz = p.Z()-v.Z();

  Double_t r = abs(dy / sqrt(dx*dx + dz*dz));

  return r;
}

TVector3
STHelixTrackFinder::FindVertex(TClonesArray *tracks, Int_t nIterations)
{
  auto TestZ = [tracks](TVector3 &v)
  {
    Double_t s = 0;
    Int_t numUsedTracks = 0;

    v.SetX(0);
    v.SetY(0);

    auto numTracks = tracks -> GetEntries();
    for (auto iTrack = 0; iTrack < numTracks; iTrack++) {
      STHelixTrack *track = (STHelixTrack *) tracks -> At(iTrack);

      TVector3 p;
      bool extrapolated = track -> ExtrapolateToZ(v.Z(), p);
      if (extrapolated == false)
        continue;

      v.SetX((numUsedTracks*v.X() + p.X())/(numUsedTracks+1));
      v.SetY((numUsedTracks*v.Y() + p.Y())/(numUsedTracks+1));

      if (numUsedTracks != 0)
        s = (double)numUsedTracks/(numUsedTracks+1)*s + (v-p).Mag()/numUsedTracks;

      numUsedTracks++;
    }

    return s;
  };

  Double_t z0 = 500;
  Double_t dz = 200;
  Double_t s0 = 1.e8;

  const Int_t numSamples = 9;
  Int_t halfOfSamples = (numSamples)/2;

  Double_t zArray[numSamples] = {0};
  for (Int_t iSample = 0; iSample < numSamples; iSample++)
    zArray[iSample] = (iSample - halfOfSamples) * dz + z0;

  for (auto z : zArray) {
    TVector3 v(0, 0, z);
    Double_t s = TestZ(v);

    if (s < s0) {
      s0 = s;
      z0 = z;
    }
  }

  while (nIterations > 0) {
    dz = dz/halfOfSamples;
    for (Int_t iSample = 0; iSample < numSamples; iSample++)
      zArray[iSample] = (iSample - halfOfSamples) * dz + z0;

    for (auto z : zArray) {
      TVector3 v(0, 0, z);
      Double_t s = TestZ(v);

      if (s < s0) {
        s0 = s;
        z0 = z;
      }
    }

    nIterations--;
  }

  TVector3 v(0, 0, z0);
  TestZ(v);

  return v;
}

void STHelixTrackFinder::SetDefaultCutScale(Double_t scale) { fDefaultScale = scale; }
void STHelixTrackFinder::SetTrackWidthCutLimits(Double_t lowLimit, Double_t highLimit)
{
  fTrackWCutLL = lowLimit;
  fTrackHCutHL = highLimit;
}
void STHelixTrackFinder::SetTrackHeightCutLimits(Double_t lowLimit, Double_t highLimit)
{
  fTrackHCutLL = lowLimit;
  fTrackHCutHL = highLimit;
}
void STHelixTrackFinder::SetClusterCutLRTB(Double_t left, Double_t right, Double_t top, Double_t bottom)
{
  fCCLeft = left;
  fCCRight = right;
  fCCTop = top;
  fCCBottom = bottom;
}

void STHelixTrackFinder::SetCylinderCut(TVector3 center, Double_t radius, Double_t zLength, Double_t margin) {
  fCutCenter = center;
  fCRadius = radius;
  fZLength = zLength;
  fCutMargin = margin;

  if (fSRadius != -1) {
    cout << "== [STHelixTrackFinder] SetSphereCut() was called before somewhere. SphereCut will be ignored." << endl;
    fSRadius = -1;
  } else if (fERadii != TVector3(-1, -1, -1)) {
    cout << "== [STHelixTrackFinder] SetEllipsoidCut() was called before somewhere. EllipsoidCut will be ignored." << endl;
    fERadii = TVector3(-1, -1, -1);
  }

  cout << "== [STHelixTrackFinder] Cylinderical cut is set with center at (" << center.X() << ", " << center.Y() << ", " << center.Z() << ") mm" << endl;
  cout << "                                                     radius of " << radius << " mm" << endl;
  cout << "                                                     length of " << zLength << " mm" << endl;
  cout << "                                                 and margin of " << margin << " mm" << endl;
}

void STHelixTrackFinder::SetSphereCut(TVector3 center, Double_t radius, Double_t margin) {
  fCutCenter = center;
  fSRadius = radius;
  fCutMargin = margin;

  if (fCRadius != -1 && fZLength != -1) {
    cout << "== [STHelixTrackFinder] SetCylinderCut() was called before somewhere. CylinderCut will be ignored." << endl;
    fCRadius = -1;
    fZLength = -1;
  } else if (fERadii != TVector3(-1, -1, -1)) {
    cout << "== [STHelixTrackFinder] SetEllipsoidCut() was called before somewhere. EllipsoidCut will be ignored." << endl;
    fERadii = TVector3(-1, -1, -1);
  }

  cout << "== [STHelixTrackFinder] Spherical cut is set with center at (" << center.X() << ", " << center.Y() << ", " << center.Z() << ") mm" << endl;
  cout << "                                                  radius of " << radius << " mm" << endl;
  cout << "                                              and margin of " << margin << " mm" << endl;
}

void STHelixTrackFinder::SetEllipsoidCut(TVector3 center, TVector3 radii, Double_t margin) {
  fCutCenter = center;
  fERadii = radii;
  fCutMargin = margin;

  if (fSRadius != -1) {
    cout << "== [STHelixTrackFinder] SetSphereCut() was called before somewhere. SphereCut will be ignored." << endl;
    fSRadius = -1;
  } else if (fCRadius != -1 && fZLength != -1) {
    cout << "== [STHelixTrackFinder] SetCylinderCut() was called before somewhere. CylinderCut will be ignored." << endl;
    fCRadius = -1;
    fZLength = -1;
  } 

  cout << "== [STHelixTrackFinder] Ellipsoidal cut is set with center at (" << center.X() << ", " << center.Y() << ", " << center.Z() << ") mm" << endl;
  cout << "                                                      radii of (" << radii.X() << ", " << radii.Y() << ", " << radii.Z() << ") mm" << endl;
  cout << "                                                 and margin of " << margin << " mm" << endl;
}

void
STHelixTrackFinder::CheckIsContinuousHits(STHitCluster *cluster)
{
  auto hits = cluster -> GetHitPtrs();

  auto isRow = cluster -> IsRowCluster();

  vector<Int_t> numbers;
  if (isRow)
    for (auto hit : *hits)
      numbers.push_back(hit -> GetLayer());
  else
    for (auto hit : *hits)
      numbers.push_back(hit -> GetRow());

  sort(numbers.begin(), numbers.end());
  numbers.erase(unique(numbers.begin(), numbers.end()), numbers.end());

  if (numbers.size() < 2) {
    cluster -> SetIsContinuousHits();
    return;
  }

  for (auto i = 0; i < numbers.size() - 1; i++)
    if (!(numbers[i] + 1 == numbers[i + 1]))
      return;

  cluster -> SetIsContinuousHits();
}

void
STHelixTrackFinder::SetClusteringAngleAndMargin(Double_t angle, Double_t margin)
{
  cout << "== [STHelixTrackFinder] Clustering angle changes at " << angle << " deg with margin " << margin << " deg." << endl;

  fClusteringAngle = angle;
  fClusteringMargin = margin;
}

Bool_t
STHelixTrackFinder::CheckBuildByLayer(STHelixTrack *helix, STHit *hit, STHit *prevHit)
{
  TVector3 q;
  Double_t alpha;
  helix -> ExtrapolateToPointAlpha(hit -> GetPosition(), q, alpha);

  auto directionChangeAngle = fClusteringAngle*TMath::DegToRad();
  auto normAlpha = TMath::Abs(std::fmod(TMath::Abs(alpha), TMath::Pi()) - TMath::Pi()/2.);
  auto isLayer = (normAlpha > TMath::Pi()/2. - directionChangeAngle);

  Double_t prevAlpha;
  if (prevHit != nullptr) {
    helix -> ExtrapolateToPointAlpha(prevHit -> GetPosition(), q, prevAlpha);

    auto prevNormAlpha = TMath::Abs(std::fmod(TMath::Abs(prevAlpha), TMath::Pi()) - TMath::Pi()/2.);
    auto isLayerPrev = (prevNormAlpha > TMath::Pi()/2. - directionChangeAngle);

    if (isLayerPrev == isLayer)
      return isLayer;

    auto margin = fClusteringMargin*TMath::DegToRad();
    auto diffNormAlpha = TMath::Abs(normAlpha - TMath::Pi()/2. + directionChangeAngle);

    if (diffNormAlpha < margin)
      return isLayerPrev;
  }

  return isLayer;
}
