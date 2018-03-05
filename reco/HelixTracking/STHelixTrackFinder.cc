#include "STHelixTrackFinder.hh"

#include <iostream>
using namespace std;

#include "STGlobal.hh"

ClassImp(STHelixTrackFinder)

class containHits {
public:
  int index;
  std::vector<STHit*> sat;//saturated hits 
  std::vector<STHit*> notsat;//not saturated hits
};

class checkIdx {
public:
  checkIdx(const int &idx) : id(idx){}
  bool operator()(const containHits a){
    if(id == a.index)
      return true;
    else
      return false;
  }
  private:
    int id;
  };
  
static void myFitFunction(Int_t& npar, Double_t* deriv, Double_t &f, Double_t* par, Int_t flag)
{
  MyFitFunction* fitFunc = MyFitFunction::Instance();
  f = fitFunc->Function(npar, deriv, f, par, flag);
}

Double_t MyFitFunction::Function(Int_t& npar, Double_t* deriv, Double_t &f, Double_t* par, Int_t flag)
{
  double chisq =0;
  vector<double> stat_array = getmean(par);
  double mean      = stat_array.at(0);
  double total_chg = stat_array.at(1);
  
  int num_elem = hits_pos_ary->size();
  for (int i=0; i<num_elem; i++)
    {
      double v  = total_chg*PRF(hits_pos_ary->at(i)-mean,par);
      if ( v != 0.0 )
	{
	  double n = hits_chg_ary->at(i);
	  chisq += pow(n-v,2)/v;
	}
      else
	{
	  //	  cout << "WARNING -- pdf is negative!!!" << endl;
	}
    }
  
 return chisq;        
}

double MyFitFunction::PRF(double x, double par[])
{
  double h_w = 4; //half width
  double x1 = x-h_w;
  double x2 = x+h_w;
  
  double sigma = 3.4;
  double x1_p = x1/(sqrt(2)*sigma);
  double x2_p = x2/(sqrt(2)*sigma);
  
  return .5*(TMath::Erf(x2_p)-TMath::Erf(x1_p));
  
};

std::vector<double> MyFitFunction::getmean(double par[])
{
  vector<double> output;
  double mean=0;
  double chg =0;
  //Here we loop over non-saturated hits
  int num_elem = hits_pos_ary->size();
  for(int i=0;i<num_elem;++i){
    mean += hits_pos_ary->at(i)*hits_chg_ary->at(i);
    chg += hits_chg_ary->at(i);
  }
  
  //Here we loop over saturated hits
  num_elem = s_hits_pos_ary->size();
  for(int i=0;i<num_elem;++i){
    mean += s_hits_pos_ary->at(i)*par[i];
    chg += par[i];
  }
  mean  = mean/chg;
  output.push_back(mean);
  output.push_back(chg);
  
  return output;
  
};

void MyFitFunction::SetAryPointers(std::vector<double> *a, std::vector<double> *a_chg, std::vector<double> *b, std::vector<double> *b_chg)
{
  hits_pos_ary = a;
  hits_chg_ary = a_chg;
  s_hits_pos_ary = b;
  s_hits_chg_ary = b_chg;

}

MyFitFunction* MyFitFunction::_instance = 0;

MyFitFunction* MyFitFunction::Instance()
{
  if (!_instance) _instance = new MyFitFunction;
  return _instance;
}

STHelixTrackFinder::STHelixTrackFinder()
{
  fFitter = new STHelixTrackFitter();
  fEventMap = new STPadPlaneMap();

  fCandHits = new std::vector<STHit*>;
  fGoodHits = new std::vector<STHit*>;
  fBadHits = new std::vector<STHit*>;
}

void 
STHelixTrackFinder::BuildTracks(TClonesArray *hitArray, TClonesArray *trackArray, TClonesArray *hitClusterArray)
{
  fTrackArray = trackArray;
  fHitClusterArray = hitClusterArray;
  fEventMap -> Clear();
  fCandHits -> clear();
  fGoodHits -> clear();
  fBadHits -> clear();

  Int_t numTotalHits = hitArray -> GetEntries();
  for (Int_t iHit = 0; iHit < numTotalHits; iHit++)
    fEventMap -> AddHit((STHit *) hitArray -> At(iHit));

  while(1)
  {
    fCandHits -> clear();
    fGoodHits -> clear();
    fBadHits -> clear();

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

    if (fSaturationOption == 1)
      De_Saturate(track);

    track -> FinalizeHits();
    HitClustering(track);

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

    section = (Int_t)(alpha/(TMath::Pi()/4));
    isRow = abs(section)%2;

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

    if (cluster -> GetClusterID() == -1)
      fHitClusterArray -> Remove(cluster);
    else if (cluster -> IsLayerCluster() && cluster -> GetDx() < 1.e-2)
      fHitClusterArray -> Remove(cluster);
    else if (cluster -> IsRowCluster() && cluster -> GetDz() < 1.e-2)
      fHitClusterArray -> Remove(cluster);
    else {
      cluster -> SetClusterID(idxCluster++);
      helix -> AddHitCluster(cluster);
    }
  }

  fHitClusterArray -> Compress();

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
  for (Int_t iSample = 0; iSample <= numSamples; iSample++)
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
    for (Int_t iSample = 0; iSample <= numSamples; iSample++)
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

void STHelixTrackFinder::SetSaturationOption(Int_t opt)
{
  fSaturationOption = opt;
  std::cout << "== [STHelixTrackFinder] DeSaturation process is turned " << (opt ? "on" : "off") << "!" << std::endl;
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
/*
double STHelixTrackFinder::PRF(double x, double par[])
{
  double h_w = 4; //half width
  double x1 = x-h_w;
  double x2 = x+h_w;
  
  double sigma = 3.4;
  double x1_p = x1/(sqrt(2)*sigma);
  double x2_p = x2/(sqrt(2)*sigma);
  
  return .5*(TMath::Erf(x2_p)-TMath::Erf(x1_p));
  
};

std::vector<double> STHelixTrackFinder:: getmean(double par[])
{
  vector<double> output;
  double mean=0;
  double chg =0;
  //Here we loop over non-saturated hits
  int num_elem = hits_pos_ary.size();
  for(int i=0;i<num_elem;++i){
    mean += hits_pos_ary.at(i)*hits_chg_ary.at(i);
    chg += hits_chg_ary.at(i);
  }
  
  //Here we loop over saturated hits
  num_elem = s_hits_pos_ary.size();
  for(int i=0;i<num_elem;++i){
    mean += s_hits_pos_ary.at(i)*par[i];
    chg += par[i];
  }
  mean  = mean/chg;
  output.push_back(mean);
  output.push_back(chg);
  
  return output;
  
};

void STHelixTrackFinder::fcn(int& npar, double* deriv, double& f, double *par, int flag)
{
  double chisq =0;
  vector<double> stat_array = getmean(par);
  double mean      = stat_array.at(0);
  double total_chg = stat_array.at(1);
  
  int num_elem = hits_pos_ary.size();
  for (int i=0; i<num_elem; i++)
    {
      double v  = total_chg*PRF(hits_pos_ary.at(i)-mean,par);
      if ( v != 0.0 )
	{
	  double n = hits_chg_ary.at(i);
	  chisq += pow(n-v,2)/v;
	}
      else
	{
	  cout << "WARNING -- pdf is negative!!!" << endl;
	}
    }
  
  f = chisq;        
};                  
*/
std::vector<double> STHelixTrackFinder::minimize(const int npar)
{
  // Initialize minuit, set initial values etc. of parameters.
  vector<double> f_par;
  TMinuit minuit(npar);
  minuit.SetPrintLevel(-1);
  minuit.SetFCN(myFitFunction);
  
  double par[npar];               // the start values
  double stepSize[npar];          // step sizes
  double minVal[npar];            // minimum bound on parameter
  double maxVal[npar];            // maximum bound on parameter
  string parName[npar];
  
  for( int i =0;i < npar; ++i)
    {
      par[i] = 4000.;            // a guess at the true value.
      stepSize[i] = 1.;       // take e.g. 0.1 of start value
      minVal[i] = 3500;   // if min and max values = 0, parameter is unbounded.  Only set bounds if you really think it's right!
      maxVal[i] = 100000;
      parName[i] = "miss charge";
    }
  
  for (int i=0; i<npar; i++)
    {
      minuit.DefineParameter(i, parName[i].c_str(),
			     par[i], stepSize[i], minVal[i], maxVal[i]);
    }
  
  // Do the minimization!
  
  minuit.Migrad();       // Minuit's best minimization algorithm
  double outpar[npar], err[npar];
  for (int i=0; i<npar; i++){
    minuit.GetParameter(i,outpar[i],err[i]);
    f_par.push_back(outpar[i]);
  }
  //       cout << endl << endl << endl;
  //       cout << "*********************************" << endl;
  //       cout << "   "<<parName[0]<<": " << outpar[0] << " +/- " << err[0] << endl;
  
  return f_par;
};

void STHelixTrackFinder::De_Saturate(STHelixTrack *track)
{
  track -> SortHitsByTimeOrder();
  auto trackHits = track -> GetHitArray();
  auto numHits = trackHits -> size();
  
  auto CheckBuildByLayer = [track](STHit *hit) {
    TVector3 q;
    Double_t alpha;
    track -> ExtrapolateToPointAlpha(hit -> GetPosition(), q, alpha);
    
    auto direction = track -> Direction(alpha);
    //angle defined from x axis not z
    Double_t angle = TMath::ATan2(TMath::Abs(direction.Z()), direction.X());
    if (angle > TMath::ATan2(1,1) && angle < TMath::ATan2(1,-1))
      return true;
    else
      return false;
    };
  
  
  std::vector<containHits> byLayer; 
  std::vector<containHits> byRow;  
  
  for (auto iHit = 0; iHit < numHits; iHit++)
    {
      auto curHit = (STHit*)trackHits -> at(iHit);
	
      bool buildByLayer = CheckBuildByLayer(curHit);//false is by layer
      
      auto row = curHit -> GetRow();
      auto layer = curHit -> GetLayer();
      auto charge = curHit -> GetCharge();
      
      //make a class for if not in vector
      containHits cont;
      cont.index = layer;
      if(charge >= 3500)//sat defined in PSA to be 3500
	cont.sat.push_back(curHit);
      else
	cont.notsat.push_back(curHit);

      //      cout<<"IN row,layer"<<row<<" "<<layer<<endl;
      //cout<<"BuildByLayer is "<<buildByLayer<<endl;
      
      if(buildByLayer)
	{//by layer 
	  auto it = std::find_if(byLayer.begin(),byLayer.end(),checkIdx(layer));
	  if(it == byLayer.end())
	    byLayer.push_back(cont);//not in vect yet; so store
	  else
	    {//in vector
	      auto idx = std::distance(byLayer.begin(),it);
	      if(charge >= 3500)
		byLayer.at(idx).sat.push_back(curHit);
	      else
		byLayer.at(idx).notsat.push_back(curHit);
	    }
	}
      else
	{//by row
	  auto it = std::find_if(byRow.begin(),byRow.end(),checkIdx(row));
	  if(it == byRow.end())
	    byRow.push_back(cont);//not in vect yet; so store
	  else
	    {//in vect
	      auto idx = std::distance(byRow.begin(),it);
	      if(charge >= 3500)
		byRow.at(idx).sat.push_back(curHit);
	      else
		byRow.at(idx).notsat.push_back(curHit);
	    }
	}
    }//end of Hit loop
  //  cout<<"GREAT YOU MADE IT TO THE END OF THE HIT LOOP"<<endl;  

  //*******
  //Section to do extrapolation
  //Now that we have the hits in a track organized into saturated
  //and non saturated hits we can now loop over the hits and extrapolate
  //the saturated hits by using the non saturated hits and doing a ChiSq min
  //*******
  
  //MAYBE add some checks here
  //Check ideas:
  //Check that there are more non-saturated pads than saturated for fit sake
  //Check if there are multiple sat pad they are next to each other
  //If not this might indicate delta electrons which of course dont follow
  // the pad response function
  //Check to see distribution is falling off from saturated pads

  //inputs for MINUIT funciton 
      
    //*********
    //Start MINUIT section
    //*********
    
    //need to put in Row PRF
    //End of MINUIT fit
    
    int nLayer = byLayer.size();
    int nRow = byRow.size();
    //    cout<<"number of layers "<<nLayer<<endl;
    //    cout<<"number of rows "<<nRow<<endl;

    MyFitFunction* fitFunc = MyFitFunction::Instance();
    std::vector<double> *hits_pos_ary_t   = new std::vector<double>;
    std::vector<double> *hits_chg_ary_t   = new std::vector<double>;
    std::vector<double> *s_hits_pos_ary_t = new std::vector<double>;
    std::vector<double> *s_hits_chg_ary_t = new std::vector<double>;

    fitFunc->SetAryPointers(hits_pos_ary_t,hits_chg_ary_t,s_hits_pos_ary_t,s_hits_chg_ary_t);    

    for(int iLayer = 0;iLayer < nLayer; ++iLayer)
      {
	containHits content = byLayer.at(iLayer);
	std::vector<STHit *>saturated = content.sat; 
	std::vector<STHit *>non_sat   = content.notsat; 
	int nSat   = saturated.size();
	int notSat = non_sat.size();
	//		cout<<"In layer "<<iLayer<<endl;
	//		cout<<"Number of sat "<<nSat<<" not sat "<<notSat<<endl;
	//Conditions of extrapolation
	if(nSat == 0)
	 continue;//if no saturated hits continue
	if(notSat == 0)
	  continue;
	//	if(!(notSat > nSat))
	//	  continue;//at least more non-sat hits than sat for extrapolation
	if(nSat>4)
	  continue;//for now try only two sat pads

	hits_pos_ary_t->clear();
	hits_chg_ary_t->clear();
	s_hits_pos_ary_t->clear();
	s_hits_chg_ary_t->clear();
	
	//Filling inputs for minimize function
	for(int iHit = 0; iHit < nSat; ++iHit)
	  {
	    STHit * hit = saturated.at(iHit);
	    double x = hit -> GetX(); //by layer clustering so look along x
	    double chg = hit -> GetCharge();
	    s_hits_pos_ary_t->push_back(x);
	    s_hits_chg_ary_t->push_back(chg);
	  }
	for(int iHit = 0; iHit < notSat; ++iHit)
	  {
	    STHit * hit = non_sat.at(iHit);
	    double x = hit -> GetX(); 
	    double chg = hit -> GetCharge();
	    hits_pos_ary_t->push_back(x);
	    hits_chg_ary_t->push_back(chg);
	 }
	
	if(s_hits_pos_ary_t->size()!=s_hits_chg_ary_t->size())
	  cout<<"ARRRAY SIZES NOT EQUAL!!!!!!!!!!!!!!!!!"<<endl;
	
	//Get predicted value in saturated pad pos
	const int npar = s_hits_pos_ary_t->size();;
	vector<double>outpar = minimize(npar);


	int nHits = s_hits_chg_ary_t->size();
	for(int iHit = 0; iHit < nHits; ++iHit)
	  {
	      //Create new hits
	    STHit * oldHit = saturated.at(iHit);
	    TVector3 oldvec = (TVector3)oldHit -> GetPosition();
	    double newChg = outpar.at(iHit);
	    if(abs(newChg-3500)<.001)//min bound 
	      continue;
	    else if(abs(newChg-100000)<.001)//max bound
	      continue;
	    /*
	    cout<<"Old hit info"<<endl;
	    cout<<"Hit ID"<<oldHit->GetHitID();
	    cout<<"Charge "<<oldHit->GetCharge()<<endl;
	    cout<<"Pos x,y,z "<<oldvec.X()<<","<<oldvec.Y()<<","<<oldvec.Z()<<endl;
	    cout<<"row, layer "<<oldHit->GetRow()<<","<<oldHit->GetLayer()<<endl;
	    */
	    STHit *newHit = new STHit(oldHit);
	    newHit->SetCharge(newChg);
	    /*
	    cout<<"New hit info"<<endl;
	    cout<<"Hit ID"<<newHit->GetHitID();
	    cout<<"Charge "<<newHit->GetCharge()<<endl;
	    cout<<"Pos x,y,z "<<newHit->GetX()<<","<<newHit->GetY()<<","<<newHit->GetZ()<<endl;
	    cout<<"row, layer "<<newHit->GetRow()<<","<<newHit->GetLayer()<<endl;
	    */
	    track -> Remove(oldHit);
	    track -> AddHit(newHit);
	    }
      }//layer loop  
  }

