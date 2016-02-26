/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFinder.hh"
#include "STDebugLogger.hh"

#include "STCorrLinearTHProx.hh"
#include "STCorrLinearTHPerp.hh"
#include "STCorrLinearTHRMS.hh"

#include "STCorrLinearTTPerp.hh"
#include "STCorrLinearTTGeo.hh"
#include "STCorrLinearTTRMS.hh"
#include "STCorrLinearTTProx.hh"

#include "TVector3.h"

#include <iostream>
#include <vector>

using namespace std;

//#define DEBUGLT

ClassImp(STLinearTrackFinder)

STLinearTrackFinder::STLinearTrackFinder()
{
  fHitQueue   = new vecHit_t;
  fTrackQueue = new vecTrk_t;
  fTrackBufferTemp = new vecTrk_t;

  fTrackClonesArray = new TClonesArray("STLinearTrack", 50);

  fFitter = new STLinearTrackFitter();

  // Linear Track - Hit Correlator ______________________________________________

  STCorrLinearTHProx *corrTHProx = new STCorrLinearTHProx (fProxXCut, fProxYCut, fProxZCut, fNumHitsCompare, fNumHitsCompareMax);
  STCorrLinearTHPerp *corrTHPerp = new STCorrLinearTHPerp (fNumHitsFit, fProxLineCut, fProxPlaneCut);
  STCorrLinearTHRMS  *corrTHRMS  = new STCorrLinearTHRMS  (fNumHitsFit, fRMSLineCut, fRMSPlaneCut);

  fCorrTH = new vecCTH_t;
  fCorrTH -> push_back(corrTHProx);
  fCorrTH -> push_back(corrTHPerp);
  fCorrTH -> push_back(corrTHRMS);

  // -----------

  corrTHProx = new STCorrLinearTHProx (fProxXCut, 2 * fProxYCut, fProxZCut, fNumHitsCompare, fNumHitsCompareMax);
  corrTHPerp = new STCorrLinearTHPerp (fNumHitsFit, fProxLineCut, 2 * fProxPlaneCut);

  fCorrTH_largeAngle = new vecCTH_t;
  fCorrTH_largeAngle -> push_back(corrTHProx);
  fCorrTH_largeAngle -> push_back(corrTHPerp);

  // -----------

  corrTHProx = new STCorrLinearTHProx (fProxXCut, 5 * fProxYCut, fProxZCut, fNumHitsCompare, fNumHitsCompareMax);

  fCorrTH_largeAngle2 = new vecCTH_t;
  fCorrTH_largeAngle2 -> push_back(corrTHProx);
  fCorrTH_largeAngle2 -> push_back(corrTHPerp);

  // -----------

  corrTHPerp = new STCorrLinearTHPerp (fNumHitsFit, fRMSLineCut, fRMSPlaneCut);

  fCorrTH_justPerp = new vecCTH_t;
  fCorrTH_justPerp -> push_back(corrTHPerp);

  // Linear Track - Track Correlator ____________________________________________

  STCorrLinearTTPerp *corrTTPerp = new STCorrLinearTTPerp (fNumHitsFit, fProxLineCut, fProxPlaneCut);
  STCorrLinearTTGeo  *corrTTGeo  = new STCorrLinearTTGeo  (fNumHitsFit, fDirectionDotCut, fNormalDotCut);
  STCorrLinearTTRMS  *corrTTRMS  = new STCorrLinearTTRMS  (fNumHitsFit, fRMSLineCut, fRMSPlaneCut);
  STCorrLinearTTProx *corrTTProx = new STCorrLinearTTProx (fNumHitsFit, fProxRCut);

  fCorrTT = new vecCTT_t;
  fCorrTT -> push_back(corrTTPerp);
  fCorrTT -> push_back(corrTTGeo);
  fCorrTT -> push_back(corrTTProx);
  fCorrTT -> push_back(corrTTRMS);
}

void 
STLinearTrackFinder::BuildTracks(STEvent *event, vecTrk_t *trackArray)
{
  Int_t numHits = event -> GetNumHits();
  if (numHits == 0)
    return;

  fHitQueue -> clear();
  fTrackQueue -> clear();
  fTrackClonesArray -> Clear();

  fTrackBufferFinal = trackArray;

  for (Int_t iHit = 0; iHit < numHits; iHit++) {
    STHit *hit = event -> GetHit(iHit);

    TVector3 p = hit -> GetPosition();
    if (TMath::Sqrt(p.X()*p.X() + p.Z()*p.Z()) < fRadialCut)
      continue;

    fHitQueue -> push_back(hit);
  }

  fNumTracks = 0;

  std::sort(fHitQueue -> begin(), fHitQueue -> end(), STHitSortXYZInv());
   
  ///////////////////////////////////////////////////////////////
  Build  (fTrackQueue, fHitQueue, fCorrTH);
  Select (fTrackQueue, fTrackQueue, fHitQueue, TMath::Pi()*1/4);
  Build  (fTrackQueue, fHitQueue, fCorrTH_justPerp, kFALSE);

  Select (fTrackQueue, fTrackBufferFinal, fHitQueue);
  Merge  (fTrackBufferFinal);
  ///////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  Build  (fTrackQueue, fHitQueue, fCorrTH_largeAngle);
  Merge  (fTrackQueue);
  Select (fTrackQueue, fTrackQueue, fHitQueue);
  Build  (fTrackQueue, fHitQueue, fCorrTH_justPerp, kFALSE);
  Select (fTrackQueue, fTrackBufferFinal, fHitQueue);
  ///////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  Build  (fTrackQueue, fHitQueue, fCorrTH_largeAngle2);
  Merge  (fTrackQueue);
  Select (fTrackQueue, fTrackQueue, fHitQueue);
  Build  (fTrackQueue, fHitQueue, fCorrTH_justPerp, kFALSE);
  Select (fTrackQueue, fTrackBufferFinal, fHitQueue);
  ///////////////////////////////////////////////////////////////

  Merge    (fTrackBufferFinal);
  SortHits (fTrackBufferFinal);
}

void 
STLinearTrackFinder::Build(vecTrk_t *trackArray, vecHit_t *hitArray, vecCTH_t *corrTH, Bool_t createNewTracks)
{
#ifdef DEBUGLT
  LOG(INFO) << "Build()" << FairLogger::endl;
#endif

  Int_t numHits = hitArray -> size();
  fTrackBufferTemp -> clear();

  for (auto track : *trackArray)
    fTrackBufferTemp -> push_back(track);

  trackArray -> clear();

  for (Int_t iHit = 0; iHit < numHits; iHit++)
  {
    Int_t idxHit = numHits - 1 - iHit;
    STHit *hit = fHitQueue -> at(idxHit);

    Double_t bestQuality = 0;
    STLinearTrack* trackCandidate = NULL;

    Int_t numTracks = fTrackBufferTemp -> size();

    for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
    {
      Int_t idxTrack = numTracks - 1 - iTrack;
      STLinearTrack *track = fTrackBufferTemp -> at(idxTrack);

      TVector3 p0 = hit -> GetPosition();
      TVector3 p1 = track -> GetHitPointerArray() -> back() -> GetPosition();

      if (p1.Z() - p0.Z() > fProxZCut * 10)
      {
        trackArray -> push_back(track);
        fTrackBufferTemp -> erase(fTrackBufferTemp -> begin() + idxTrack);
        continue;
      }

      Double_t quality = 0;
      Bool_t survive = kFALSE;

      for (auto correlator : *corrTH) 
      {
        correlator -> Correlate(track, hit, survive, quality);
        if (survive == kFALSE) 
          break;
      }

      if (survive == kTRUE)
      {
        if (quality > bestQuality) 
        {
          bestQuality = quality;
          trackCandidate = track;
        }
      }
    }
    
    if (trackCandidate != NULL){
      trackCandidate -> AddHit(hit);
      hitArray -> erase(hitArray -> begin() + idxHit);
      fFitter -> FitAndSetTrack(trackCandidate);
    }
    else if (createNewTracks == kTRUE) {
      Int_t index = fTrackClonesArray -> GetEntriesFast();
      STLinearTrack* track = new ((*fTrackClonesArray)[index]) STLinearTrack(fNumTracks++, hit);
      fTrackBufferTemp -> push_back(track);
      hitArray -> erase(hitArray -> begin() + idxHit);
    }
  }

  for (auto track : *fTrackBufferTemp)
    trackArray -> push_back(track);

  fTrackBufferTemp -> clear();
}

void
STLinearTrackFinder::Merge(vecTrk_t *trackArray)
{
#ifdef DEBUGLT
  fLogger -> Info(MESSAGE_ORIGIN,"Merge()");
#endif
  Int_t numTracks = trackArray -> size();

  for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
  {
    Int_t idxTrack = numTracks - 1 - iTrack;
    auto track = trackArray -> at(idxTrack);

    Double_t bestQuality = 0;
    STLinearTrack* trackCandidate = NULL;

    Int_t numTracksLeft = idxTrack;

    for (Int_t iTrackCompare = 0; iTrackCompare < numTracksLeft; iTrackCompare++)
    {
      Int_t idxTrackCompare = numTracksLeft - 1 - iTrackCompare;
      auto trackCompare = trackArray -> at(idxTrackCompare);

      Double_t quality = 0;
      Bool_t survive = kFALSE;
      for (auto correlator : *fCorrTT) 
      {
        correlator -> Correlate(track, trackCompare, survive, quality);
        if (survive == kFALSE) 
          break;
      }

      if (survive == kTRUE)
      {
        if (quality > bestQuality) 
        {
          bestQuality = quality;
          trackCandidate = trackCompare;
        }
      }
    }
    
    if (trackCandidate != NULL)
    {
      fFitter -> MergeAndSetTrack(trackCandidate, track);
      trackArray -> erase(trackArray -> begin() + idxTrack);
    }
  }
}

void
STLinearTrackFinder::Select(vecTrk_t *trackArray, vecTrk_t *trackArray2, vecHit_t *hitArray, Double_t thetaCut)
{
#ifdef DEBUGLT
  fLogger -> Info(MESSAGE_ORIGIN,"Select()");
#endif

  TVector3 pointingZ(0,0,1);
  Double_t cosineCut = TMath::Cos(thetaCut);

  fTrackBufferTemp -> clear();

  for (auto track : *trackArray)
  {
    if (track -> GetNumHits() < fNumHitsTrackCut) {
      if (track -> GetNumHits() > fNumHitsVanishCut) 
        ReturnHits(track, hitArray);
      continue; 
    }

    if (track -> IsFitted() == kFALSE)
      fFitter -> FitAndSetTrack(track); 

    if (track -> GetRMSLine() > 2 * fRMSLineCut || track -> GetRMSPlane() > 2 * fRMSPlaneCut) {
      ReturnHits(track, hitArray);
      continue; 
    }

    if (pointingZ.Dot(track -> GetDirection()) < cosineCut) {
      ReturnHits(track, hitArray);
      continue;
    }

    fTrackBufferTemp -> push_back(track);
  }

  trackArray -> clear();

  for (auto track : *fTrackBufferTemp)
    trackArray2 -> push_back(track);

  fTrackBufferTemp -> clear();
}

void 
STLinearTrackFinder::SortHits(vecTrk_t *trackArray)
{
  Int_t numTracks = trackArray -> size();
  for(Int_t iTrack = 0; iTrack < numTracks; iTrack++) 
  {
    STLinearTrack* track = trackArray -> at(iTrack);
    fFitter -> SortHits(track);
  }
}

void
STLinearTrackFinder::ReturnHits(STLinearTrack *track, vecHit_t *hitArray)
{
  vecHit_t *hitsFromTrack = track -> GetHitPointerArray();

  for (auto hit : *hitsFromTrack)
    hitArray -> push_back(hit);
}
