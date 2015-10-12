/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFinder.hh"
#include "STDebugLogger.hh"

#include "TVector3.h"
#include "TMath.h"

#include <iostream>

STLinearTrackFinder::STLinearTrackFinder()
{
  fPrimaryVertex = TVector3(0, -213.3, -35.2);
  fProxCut = 1000;
  fRMSCut = 1000;

  fHitBuffer = new std::vector<STHit*>;

  fLTFitter = new STLinearTrackFitter();
}

STLinearTrackFinder::~STLinearTrackFinder()
{
}

void STLinearTrackFinder::BuildTracks(STEvent* event,
                                      std::vector<STLinearTrack*> *trackBuffer)
{
  fHitBuffer -> clear();
  fTrackBuffer = trackBuffer;
  Int_t nHits = event -> GetNumHits();
  if (nHits == 0)
    return;

  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    STHit *hit = new STHit(event -> GetHit(iHit));
    fHitBuffer -> push_back(hit);
  }
  
  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), STLTFHitSortTheta());

  STHit *hit = fHitBuffer -> at(0);
  CreateNewTrack(hit);

  for (Int_t iHit = 1; iHit < nHits; iHit++) 
  {
    hit = fHitBuffer -> at(iHit);

    Int_t nTracks = fTrackBuffer -> size();
    std::cout << iHit << " / " << nHits << " : " << nTracks << std::endl;

    Bool_t survive;

    for(Int_t iTrack = 0; iTrack < nTracks; iTrack++) 
    {
      STLinearTrack* track = fTrackBuffer -> at(iTrack);

      if (track -> GetNumHits() < 6)
        survive = ProximityCorrelator(track, hit);
      else 
      {
        survive = PerpDistCorrelator(track, hit);

        if (survive == kTRUE)
          survive = QualityTest(track, hit);
      }

      if (survive == kTRUE)
      {
        track -> AddHit(hit);
        fLTFitter -> FitAndSetTrack(track);
        break;
      }
    }
    if (survive == kFALSE) 
      CreateNewTrack(hit);
  }
}

Bool_t 
STLinearTrackFinder::ProximityCorrelator(STLinearTrack* track, STHit* hit)
{
  Int_t n = track -> GetNumHits();
  STHit *hitFT = track -> GetHit(track -> GetNumHits() - 1);

  TVector3 posT = hitFT -> GetPosition();
  TVector3 posH = hit -> GetPosition();

  Double_t dX = posT.X()-posH.X();
  Double_t dY = posT.Y()-posH.Y();
  Double_t dZ = posT.Z()-posH.Z();

  Double_t distance = TMath::Sqrt(dX*dX + dY*dY + dZ*dZ);

  STDebugLogger::Instance() -> FillHist1("distHH",distance,100,0,100);
  //std::cout << "distHH: " << distance << std::endl;


  if (distance < fProxCut) 
    return kTRUE;
  else 
    return kFALSE;
}

Bool_t 
STLinearTrackFinder::PerpDistCorrelator(STLinearTrack* track, STHit* hit)
{
  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t distance = (fLTFitter -> Perp(track, hit)).Mag();

  STDebugLogger::Instance() -> FillHist1("distHT",distance,100,0,500);
  //std::cout << "distHT: " << distance << std::endl;

  if (distance < fProxCut) 
    return kTRUE;
  else 
    return kFALSE;
}

Bool_t 
STLinearTrackFinder::QualityTest(STLinearTrack* track, STHit* hit)
{
  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t rms = fLTFitter -> Fit(track, hit);
  STDebugLogger::Instance() -> FillHist1("rms",rms,100,0,500);

  //std::cout << "rms: " << rms << std::endl;

  if (rms < fRMSCut) 
    return kTRUE;
  else 
    return kFALSE;

  return kTRUE;
}

void 
STLinearTrackFinder::CreateNewTrack(STHit* hit)
{
  STLinearTrack *track = new STLinearTrack();
  track -> AddHit(hit);
  track -> SetTrackID(fTrackBuffer -> size());
  fTrackBuffer -> push_back(track);
}
