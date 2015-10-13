/**
 * @author JungWoo Lee
 */

#include "FairRuntimeDb.h"
#include "FairRun.h"

#include "STLinearTrackFinder.hh"
#include "STDebugLogger.hh"
#include "STDigiPar.hh"

#include "TVector3.h"
#include "TMath.h"

#include <iostream>

//#define DEBUGCOUTAAAAAH
//#define DEBUGLOGGERAAAH

STLinearTrackFinder::STLinearTrackFinder()
{
  FairRun *run = FairRun::Instance();
  FairRuntimeDb *db = run -> GetRuntimeDb();
  STDigiPar* par = (STDigiPar *) db -> getContainer("STDigiPar");

  Double_t tbTime = par -> GetTBTime();
  Double_t driftVelocity = par -> GetDriftVelocity();
  Double_t padSizeX = par -> GetPadSizeX();
  Double_t padSizeZ = par -> GetPadSizeZ();

  Double_t yTb  = tbTime * driftVelocity/100.;
  Double_t cYTb = 2.5;

  fProxHHXCut = 1.1 * padSizeX;
  fProxHHZCut = 1.1 * padSizeZ;
  fProxHHYCut = 1.1 * cYTb*yTb;

  fProxHHXZCut = 1.1 * TMath::Sqrt(padSizeX*padSizeX + padSizeZ*padSizeZ);
  fProxHHCut   = 2 * TMath::Sqrt(fProxHHXZCut*fProxHHXZCut + cYTb*yTb*cYTb*yTb);

  fProxHTCut = 15;
  fProxHPCut = 1.5;


  fRMSLineCut = 6;
  fRMSPlaneCut = 2;

  fPrimaryVertex = TVector3(0, -213.3, -35.2);

  fHitBuffer = new std::vector<STHit*>;
  fTrackBufferQueue = new std::vector<STLinearTrack*>;
  fLTFitter = new STLinearTrackFitter();

  fNumHHCompare = 8;
  fMinNumHitFitLine = 5;
  fMinNumHitFitPlane = 5;
  fMinNumHitCut = 30;
}

STLinearTrackFinder::~STLinearTrackFinder()
{
}

void STLinearTrackFinder::BuildTracks(STEvent* event,
                                      std::vector<STLinearTrack*> *trackBuffer)
{
  fHitBuffer -> clear();
  fTrackBufferQueue -> clear();

  fTrackBuffer = trackBuffer;

  Int_t nHits = event -> GetNumHits();
  if (nHits == 0)
    return;

  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    STHit *hit = new STHit(event -> GetHit(iHit));
    fHitBuffer -> push_back(hit);
  }

  //**********************************************************************/
  
  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), STHitSortX());
  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), STHitSortZ());

  BuildTracks();
  //SortHits(fTrackBufferQueue);
  MergeTracks(fTrackBufferQueue);
  ChooseTracks();

  //**********************************************************************/

  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), STHitSortZ());
  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), STHitSortX());

  BuildTracks();
  //SortHits(fTrackBufferQueue);
  MergeTracks(fTrackBufferQueue);
  ChooseTracks();

  //**********************************************************************/

  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), STHitSortZ());
  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), STHitSortY());

  fProxHHYCut *= 2.5;

  BuildTracks();
  SortHits(fTrackBufferQueue);
  MergeTracks(fTrackBufferQueue);
  ChooseTracks();

  //**********************************************************************/

  MergeTracks(fTrackBuffer);
  SortHits(fTrackBuffer);
}

void 
STLinearTrackFinder::BuildTracks()
{
  Bool_t survive = kFALSE;
  Int_t nHits = fHitBuffer -> size();
  for (Int_t iHit = 0; iHit < nHits; iHit++) 
  {
    STHit *hit = fHitBuffer -> at(iHit); ////////

    Int_t nTracks = fTrackBufferQueue -> size();
    for(Int_t iTrack = 0; iTrack < nTracks; iTrack++) 
    {
      STLinearTrack* track = fTrackBufferQueue -> at(iTrack); /////////

#ifdef DEBUGCOUTAAAAAH
      std::cout << "COOOOOOO: " << iHit << " " << iTrack << " ... " << track -> GetNumHits() << std::endl;
#endif
      survive = Correlate(track, hit); ///////////

      if (survive == kTRUE) {
        track -> AddHit(hit);
        fLTFitter -> FitAndSetTrack(track);
        break;
      }
    }
    if (survive == kFALSE) 
    {
      //if (iHit !=0) 
        //return;
      CreateNewTrack(hit);
    }
  }
}

void 
STLinearTrackFinder::SortHits(std::vector<STLinearTrack*> *trackBuffer)
{
  Int_t nTracks = trackBuffer -> size();
  for(Int_t iTrack = 0; iTrack < nTracks; iTrack++) 
  {
    STLinearTrack* track = trackBuffer -> at(iTrack);
    fLTFitter -> SortHits(track);
  }
}

void 
STLinearTrackFinder::ChooseTracks()
{
  fHitBuffer -> clear();

  Int_t nTracks = fTrackBufferQueue -> size();
  for(Int_t iTrack = 0; iTrack < nTracks; iTrack++) 
  {
    STLinearTrack* track = fTrackBufferQueue -> back();

    Int_t nHitsInTrack = track -> GetNumHits();

#ifdef DEBUGCOUTAAAAAH
    std::cout << "CHOOOOOSE: " << nHitsInTrack << std::endl;
#endif
#ifdef DEBUGLOGGERAAAH
    STDebugLogger::Instance() -> FillHist1("numHits",nHitsInTrack,200,0,200);
#endif

    if (nHitsInTrack < fMinNumHitCut) {
      for (Int_t iHit = 0; iHit < nHitsInTrack; iHit++)
        fHitBuffer -> push_back(track -> GetHit(iHit));
    }
    else
      fTrackBuffer -> push_back(track);

    fTrackBufferQueue -> pop_back();
  }
}

void
STLinearTrackFinder::MergeTracks(std::vector<STLinearTrack*> *trackBuffer)
{
  Int_t nTracks = trackBuffer -> size();
  for(Int_t iTrack = 0; iTrack < nTracks; iTrack++) 
  {
    Int_t iIdxTrack = nTracks - 1 - iTrack;
    STLinearTrack* tracki = trackBuffer -> at(iIdxTrack); ////////

    Int_t nTracksCompare = iIdxTrack;
    for (Int_t jTrack = 0; jTrack < nTracksCompare; jTrack++)
    {
      Int_t jIdxTrack = nTracksCompare - 1 - jTrack;
      STLinearTrack* trackj = trackBuffer -> at(jIdxTrack); ////////

      Double_t rmsL, rmsP;
      fLTFitter -> Fit(trackj, tracki, rmsL, rmsP);

      if (rmsL < fRMSLineCut && rmsP < fRMSPlaneCut) 
      {
        // be careful when using erase() of vector
        // merge tracki INTO trackj
        // add tracki to trackj and erase tracki 
        fLTFitter -> MergeAndSetTrack(trackj, tracki);
        trackBuffer -> erase(trackBuffer -> begin() + iIdxTrack);
        break;
      }
    }
  }
}

Bool_t
STLinearTrackFinder::Correlate(STLinearTrack* track, STHit* hit)
{
  Bool_t survive;

               survive = CorrelateProximityXYZ(track, hit);
  if (survive) survive = CorrelatePerpPlane(track, hit);
  if (survive) survive = CorrelatePerpLine(track, hit);
  if (survive) survive = RMSTest(track, hit);

  return survive;
}

Bool_t 
STLinearTrackFinder::CorrelateProximityXYZ(STLinearTrack* track, STHit* hit)
{
  Int_t nHits = track -> GetNumHits();
  Int_t nHitIter = nHits;

  if (fNumHHCompare <= nHits)
    nHitIter = fNumHHCompare;
  else
    return CorrelateProximity(track, hit);


  TVector3 posH = hit -> GetPosition();

  //std::cout << std::endl;
  for (Int_t iHit = 0; iHit < nHitIter; iHit++)
  {
    STHit *hitT = track -> GetHit(nHits - 1 - iHit);
    TVector3 posT = hitT -> GetPosition();

    Double_t dX = posT.X()-posH.X();
    Double_t dZ = posT.Z()-posH.Z();
    Double_t dY = posT.Y()-posH.Y();

#ifdef DEBUGCOUTAAAAAH
    std::cout << " == coprox: " << iHit << ", "<< dX << " " << dY << " " << dZ << " / "
              << fProxHHXCut << " "<< fProxHHYCut << " "<< fProxHHZCut << " "<< std::endl;
#endif

    if (dX < 0) dX *= -1;
    if (dY < 0) dY *= -1;
    if (dZ < 0) dZ *= -1;

#ifdef DEBUGLOGGERAAAH
    STDebugLogger::Instance() -> FillHist1("dX",dX,200,0,50);
    STDebugLogger::Instance() -> FillHist1("dY",dY,200,0,50);
    STDebugLogger::Instance() -> FillHist1("dZ",dZ,200,0,50);
#endif

    if (dX <= fProxHHXCut && dZ <= fProxHHZCut && dY <= fProxHHYCut) 
      return kTRUE;
  }

  return kFALSE;
}

Bool_t 
STLinearTrackFinder::CorrelateProximity(STLinearTrack* track, STHit* hit)
{
  Int_t nHits = track -> GetNumHits();
  Int_t nHitIter = nHits;

  if (fNumHHCompare <= nHits)
    nHitIter = fNumHHCompare;

  TVector3 posH = hit -> GetPosition();

  for (Int_t iHit = 0; iHit < nHitIter; iHit++)
  {
    STHit *hitT = track -> GetHit(nHits - 1 - iHit);
    TVector3 posT = hitT -> GetPosition();

    Double_t dX = posT.X()-posH.X();
    Double_t dY = posT.Y()-posH.Y();
    Double_t dZ = posT.Z()-posH.Z();

    Double_t distance = TMath::Sqrt(dX*dX + dY*dY + dZ*dZ);
#ifdef DEBUGCOUTAAAAAH
    std::cout << " == D3: " << iHit << ", "
              << dX <<" "<< dY <<" "<< dZ <<" "<< distance << " / " << fProxHHCut<< std::endl;
#endif

    if (distance < 1.5 * fProxHHCut) 
      return kTRUE;
  }

  return kFALSE;
}

Bool_t 
STLinearTrackFinder::CorrelatePerpPlane(STLinearTrack* track, STHit* hit)
{
  if (track -> GetNumHits() <= fMinNumHitFitPlane) 
    return kTRUE;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t perp = (fLTFitter -> PerpToPlane(track, hit)).Mag();
#ifdef DEBUGCOUTAAAAAH
  std::cout << " == CPP: " << perp << " / " << fProxHPCut << std::endl;
#endif

#ifdef DEBUGLOGGERAAAH
  STDebugLogger::Instance() -> FillHist1("distHTP",perp,200,0,50);
#endif

  if (perp < fProxHPCut) 
    return kTRUE;
  else 
    return kFALSE;
}

Bool_t 
STLinearTrackFinder::CorrelatePerpLine(STLinearTrack* track, STHit* hit)
{
  if (track -> GetNumHits() <= fMinNumHitFitLine) 
    return kTRUE;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t perp = (fLTFitter -> Perp(track, hit)).Mag();
#ifdef DEBUGCOUTAAAAAH
  std::cout << " == CPL: " << perp << " / " << fProxHTCut << std::endl;
#endif

#ifdef DEBUGLOGGERAAAH
  STDebugLogger::Instance() -> FillHist1("distHTL",perp,100,0,50);
#endif

  if (perp < fProxHTCut) 
    return kTRUE;
  else 
    return kFALSE;
}

Bool_t 
STLinearTrackFinder::RMSTest(STLinearTrack* track, STHit* hit)
{
  if (track -> GetNumHits() <= fMinNumHitFitPlane || 
      track -> GetNumHits() <= fMinNumHitFitLine) 
    return kTRUE;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t rmsL, rmsP;
  fLTFitter -> Fit(track, hit, rmsL, rmsP);
#ifdef DEBUGLOGGERAAAH
  STDebugLogger::Instance() -> FillHist1("rmsL",rmsL,500,0,10);
  STDebugLogger::Instance() -> FillHist1("rmsP",rmsP,500,0,10);
#endif
#ifdef DEBUGCOUTAAAAAH
  std::cout << " == rmsL,P: " << rmsL << " " << rmsP << std::endl;
#endif

  if (rmsL < fRMSLineCut && rmsP < fRMSPlaneCut)
    return kTRUE;
  else 
    return kFALSE;
}

void 
STLinearTrackFinder::CreateNewTrack(STHit* hit)
{
#ifdef DEBUGCOUTAAAAAH
  std::cout << std::endl << "create" << std::endl;
#endif
  STLinearTrack *track = new STLinearTrack();
  track -> AddHit(hit);
  track -> SetTrackID(fTrackBufferQueue -> size());
  fTrackBufferQueue -> push_back(track);
}
