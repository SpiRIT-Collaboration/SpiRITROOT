#include "STHelixTrackFinder.hh"

#include <iostream>
using namespace std;

ClassImp(STHelixTrackFinder)

STHelixTrackFinder::STHelixTrackFinder()
{
  fFitter = new STHelixTrackFitter();
  fEventMap = new STPadPlaneMap();

  fCandHits = new std::vector<STHit*>;
  fGoodHits = new std::vector<STHit*>;
  fBadHits = new std::vector<STHit*>;
}

void 
STHelixTrackFinder::BuildTracks(TClonesArray *hitArray, TClonesArray *trackArray)
{
  fTrackArray = trackArray;
  fEventMap -> Clear();
  fCandHits -> clear();
  fGoodHits -> clear();
  fBadHits -> clear();

  Int_t numTotalHits = hitArray -> GetEntries();
  for (Int_t iHit = 0; iHit < numTotalHits; iHit++) {
    auto hit = (STHit *) hitArray -> At(iHit);
    auto p = hit -> GetPosition() - TVector3(0, -213.3, -89);
    if (sqrt(p.X()*p.X() + p.Z()*p.Z() + 3*p.Y()*p.Y()) < 150)
      continue;
    fEventMap -> AddHit(hit);
  }

  while(1)
  {
    fCandHits -> clear();
    fGoodHits -> clear();
    fBadHits -> clear();

    STHelixTrack *track = NewTrack();
    if (track == nullptr)
      break;

    bool survive = TrackInitialization(track);

    if (survive) {
      TrackContinuum(track);
      TrackExtrapolation(track);
      survive = TrackQualityCheck(track);
      if (survive) TrackConfirmation(track);
    }

    for (auto badHit : *fBadHits)
      fEventMap -> AddHit(badHit);
    fBadHits -> clear();

    if (track -> TrackLength() < 150) 
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

bool
STHelixTrackFinder::TrackInitialization(STHelixTrack *track)
{
  fEventMap -> PullOutNeighborHits(fGoodHits, fCandHits);
  fGoodHits -> clear();

  Int_t numCandHits = fCandHits -> size();;

  while (numCandHits != 0) {
    sort(fCandHits -> begin(), fCandHits -> end(), STHitSortCharge());

    for (Int_t iHit = 0; iHit < numCandHits; iHit++) {
      STHit *candHit = fCandHits -> back();
      fCandHits -> pop_back();

      Double_t quality = CorrelateSimple(track, candHit);

      if (quality > 0) {
        fGoodHits -> push_back(candHit);
        track -> AddHit(candHit);

        if (track -> GetNumHits() > 10) {
          if (track -> GetNumHits() > 15) {
            for (auto candHit2 : *fCandHits)
              fEventMap -> AddHit(candHit2);
            fCandHits -> clear();
            break;
          }
          fFitter -> Fit(track);
          if (track -> TrackLength() > 2 * track -> GetRMSW())
            return true;
        }
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

  bool buildHead = true;
  Double_t extrapolationLength = 0;
  while (AutoBuildByExtrapolation(track, buildHead, extrapolationLength)) {}

  buildHead = !buildHead;
  extrapolationLength = 0;
  while (AutoBuildByExtrapolation(track, buildHead, extrapolationLength)) {}

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
STHelixTrackFinder::AutoBuildByInterpolation(STHelixTrack *track, bool &tailToHead, Double_t &extrapolationLength)
{
  TVector3 p;
  if (tailToHead) p = track -> InterpolateByLength(extrapolationLength);
  else            p = track -> InterpolateByLength(track -> TrackLength() - extrapolationLength);

  return AutoBuildAtPosition(track, p, tailToHead, extrapolationLength);
}

bool 
STHelixTrackFinder::AutoBuildAtPosition(STHelixTrack *track, TVector3 p, bool &tailToHead, Double_t &extrapolationLength)
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
      quality = Correlate(track, candHit);

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

  return true;
}

bool
STHelixTrackFinder::TrackConfirmation(STHelixTrack *track)
{
  auto tailToHead = true;
  if (track -> PositionAtTail().Z() > track -> PositionAtHead().Z())
    tailToHead = false;

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
      auto helicity = track -> Helicity();
      fFitter -> Fit(track);
      if (helicity != track -> Helicity())
        tailToHead = !tailToHead;
    }

    auto dLength = std::abs(lCur - lPre);
    extrapolationLength = 10;
    while(dLength > 0 && AutoBuildByInterpolation(track, tailToHead, extrapolationLength)) { dLength -= 10; }
  }

  extrapolationLength = 0;
  while (AutoBuildByExtrapolation(track, tailToHead, extrapolationLength)) {}

  return true;
}

Int_t STHelixTrackFinder::CheckHitOwner(STHit *hit)
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
STHelixTrackFinder::Correlate(STHelixTrack *track, STHit *hit)
{
  Double_t scale = 2;
  Double_t trackLength = track -> TrackLength();
  if (trackLength < 500.)
    scale = 2.5 + .5*(500. - trackLength)/500.;

  Double_t rmsWCut = scale * track -> GetRMSW();
  if (rmsWCut < 12) rmsWCut = 12;
  if (rmsWCut > 24) rmsWCut = 24;

  Double_t rmsHCut = scale * track -> GetRMSH();
  if (rmsHCut < 3) rmsHCut = 3;
  if (rmsHCut > 10) rmsHCut = 10;

  auto qHead = track -> Map(track -> PositionAtHead());
  auto qTail = track -> Map(track -> PositionAtTail());
  auto q = track -> Map(hit -> GetPosition());

  auto LengthAlphaCut = [track](Double_t dLength) {
    if (dLength > 0) {
      if (dLength > .5*track -> TrackLength() || std::abs(track -> AlphaByLength(dLength)) > .3*TMath::Pi())
        return true;
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

  Double_t dr = std::abs(q.X());
  Double_t quality = 0;
  if (dr < rmsWCut && std::abs(q.Y()) < rmsHCut)
    quality = sqrt((dr-rmsWCut)*(dr-rmsWCut)) / rmsWCut;

  return quality;
}

Double_t 
STHelixTrackFinder::CorrelateSimple(STHelixTrack *track, STHit *hit)
{
  if (hit -> GetNumTrackCands() != 0)
    return 0;

  Double_t quality = 0;

  auto trackHits = track -> GetHitArray();
  for (auto trackHit : *trackHits) {
    Double_t dx = std::abs(hit->GetX() - trackHit->GetX());
    Double_t dy = std::abs(hit->GetY() - trackHit->GetY());
    Double_t dz = std::abs(hit->GetZ() - trackHit->GetZ());

    Double_t r = TangentOfMaxDipAngle(trackHit);
    Double_t highcut = 1.5 * r * sqrt(dx*dx + dz*dz);
    //Double_t lowcut  = 0.2 * r * sqrt(dx*dx + dz*dz);
    if (highcut < 5) 
      highcut = 5;

    if (dy > highcut)
      continue;
    else {
      quality = sqrt((dy-highcut)*(dy-highcut)) / highcut;
      break;
    }
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

  Double_t r = std::abs(dy / sqrt(dx*dx + dz*dz));

  return r;
}
