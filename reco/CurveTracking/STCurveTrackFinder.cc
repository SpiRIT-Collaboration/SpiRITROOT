/**
 * @brief  Curve track finder class
 *
 * @author JungWoo Lee
 */

#include "STCurveTrackFinder.hh"
#include "STCorrCurveTHProx.hh"
#include "STCorrCurveTHPerp.hh"

#include <iostream>
using namespace std;

ClassImp(STCurveTrackFinder)

STCurveTrackFinder::STCurveTrackFinder()
{
  fFitter = new STCurveTrackFitter();
  fFitter -> SetNumHitsAtHead(fNumHitsAtHead);

  fHitBuffer = new std::vector<STHit *>;

  STCorrCurveTHProx *corrTHProx = new STCorrCurveTHProx(fProxXCut, fProxYCut, fProxZCut, fNumHitsCompare, fNumHitsCompareMax);
  STCorrCurveTHPerp *corrTHPerp = new STCorrCurveTHPerp(fNumHitsFit, fProxLineCut, fProxPlaneCut);

  fTrackHitCorrelators = new std::vector<STCorrCurveTH *>;
  fTrackHitCorrelators -> push_back(corrTHProx);
  fTrackHitCorrelators -> push_back(corrTHPerp);

  fTrackArrayTemp1 = new TClonesArray("STCurveTrack", 20);
  fTrackArrayTemp2 = new TClonesArray("STCurveTrack", 20);
}

void
STCurveTrackFinder::BuildTracks(STEvent *event, TClonesArray *trackArray)
{
  fTrackArrayTemp1 -> Clear("C");
  fTrackArrayTemp2 -> Clear("C");

  Init(event, fTrackArrayTemp1);
  while (AnaNextHit()) {}

  fTrackArrayAna = fTrackArrayTemp2;
  Int_t numTracks = fTrackArrayTemp1 -> GetEntriesFast();

  fNumTracks = 0;
  for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
  {
    STCurveTrack *track = (STCurveTrack *) fTrackArrayTemp1 -> At(iTrack);

    fTrackArrayAna -> Clear();
    fHitBufferAna = track -> GetHitPointerArray();

    while (AnaNextHit()) {}
    trackArray -> AbsorbObjects(fTrackArrayAna);
  }
}

void
STCurveTrackFinder::BuildTracks(TClonesArray *hitArray, TClonesArray *trackArray)
{
  fTrackArrayTemp1 -> Clear("C");
  fTrackArrayTemp2 -> Clear("C");

  Init(hitArray, fTrackArrayTemp1);
  while (AnaNextHit()) {}

  fTrackArrayAna = fTrackArrayTemp2;
  Int_t numTracks = fTrackArrayTemp1 -> GetEntriesFast();

  fNumTracks = 0;
  for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
  {
    STCurveTrack *track = (STCurveTrack *) fTrackArrayTemp1 -> At(iTrack);

    fTrackArrayAna -> Clear();
    fHitBufferAna = track -> GetHitPointerArray();

    while (AnaNextHit()) {}
    trackArray -> AbsorbObjects(fTrackArrayAna);
  }
}

void
STCurveTrackFinder::Init(STEvent *event, TClonesArray *trackArray)
{
  fTrackArrayAna = trackArray;
  fHitBuffer -> clear();

  Int_t numHits = event -> GetNumHits();
  if (numHits == 0)
    return;

  for (Int_t iHit = 0; iHit < numHits; iHit++) {
    STHit *hit = event -> GetHit(iHit);

    TVector3 p = hit -> GetPosition() - fVertex;
    if (sqrt(p.X()*p.X() + p.Z()*p.Z() + p.Y()*p.Y()) < fRadialCut)
      continue;

    fHitBuffer -> push_back(hit);
  }

  STHitSortRInvFromP sorting(fVertex);
  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), sorting);

  fHitBufferAna = fHitBuffer;

  fNumTracks = 0;
}

void
STCurveTrackFinder::Init(TClonesArray *hitArray, TClonesArray *trackArray)
{
  fTrackArrayAna = trackArray;
  fHitBuffer -> clear();

  Int_t numHits = hitArray -> GetEntriesFast();
  if (numHits == 0)
    return;

  for (Int_t iHit = 0; iHit < numHits; iHit++) {
    STHit *hit = (STHit *) hitArray -> At(iHit);

    TVector3 p = hit -> GetPosition() - fVertex;
    if (sqrt(p.X()*p.X() + p.Z()*p.Z() + p.Y()*p.Y()) < fRadialCut)
      continue;

    fHitBuffer -> push_back(hit);
  }

  STHitSortRInvFromP sorting(fVertex);
  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), sorting);

  fHitBufferAna = fHitBuffer;

  fNumTracks = 0;
}

Bool_t
STCurveTrackFinder::AnaNextHit()
{
  if (fHitBufferAna -> size() == 0)
    return kFALSE;

  STHit *hit = fHitBufferAna -> back();
  fHitBufferAna -> pop_back();

  Double_t bestQuality = 0;
  STCurveTrack* trackCandidate = nullptr;

  Int_t numCorr = fTrackHitCorrelators -> size();
  Int_t numTracks = fTrackArrayAna -> GetEntriesFast();

  for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
  {
    Int_t idxTrack = numTracks - 1 - iTrack;
    STCurveTrack *track = (STCurveTrack *) fTrackArrayAna -> At(idxTrack);

#ifdef DEBUG_CURVE
  cout << "Correlate Track-" << track -> GetTrackID() << " And "
       << "Hit-" << hit -> GetHitID() << endl;
#endif

    Double_t quality = 0;
    Bool_t survive = kFALSE;

    for (auto corr : *fTrackHitCorrelators)
    {
      corr -> Correlate(track, hit, survive, quality);
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
#ifdef DEBUG_CURVE
  cout << "Qulity: " << quality << endl;
  cout << endl;
#endif
  }

  if (trackCandidate != nullptr)
  {
#ifdef DEBUG_CURVE
    cout << "Add Hit-" << hit -> GetHitID() 
         << " to Track-" << trackCandidate -> GetTrackID() << endl;
#endif
    trackCandidate -> AddHit(hit);

    if (trackCandidate -> GetNumHits() >= fNumHitsFit)
      fFitter -> FitAndSetCurveTrack(trackCandidate);
  }
  else
  {
    STCurveTrack* track = new ((*fTrackArrayAna)[fTrackArrayAna -> GetEntriesFast()]) STCurveTrack(fNumTracks, hit);
#ifdef DEBUG_CURVE
    cout << "New Track-" << track -> GetTrackID() << endl;
#endif
    fNumTracks++;
  }

#ifdef DEBUG_CURVE
  cout << endl;
#endif

  return kTRUE;
}
