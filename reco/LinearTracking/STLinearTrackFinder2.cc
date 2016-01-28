/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFinder2.hh"
#include "STDebugLogger.hh"

#include "TMath.h"
#include "TVector3.h"

ClassImp(STLinearTrackFinder2)

#define DEBUG_cout_build
#define DEBUG_cout_position
#define DEBUG_cout_merge
#define DEBUG_cout_choose
#define DEBUG_cout_status
#define DEBUG_logger

STLinearTrackFinder2::STLinearTrackFinder2()
{
  fPrimaryVertex = TVector3(0, -213.3, -35.2);

  fHitQueue   = new std::vector<STHit*>;
  fTrackQueue = new std::vector<STLinearTrack*>;

  fLTFitter = new STLinearTrackFitter();
}

void STLinearTrackFinder2::BuildTracks(STEvent* event,
                                      std::vector<STLinearTrack*> *trackBuffer)
{
  fHitQueue -> clear();
  fTrackQueue -> clear();

  fTrackBuffer = trackBuffer;

  Int_t numHits = event -> GetNumHits();
  if (numHits == 0)
    return;

  for (Int_t iHit = 0; iHit < numHits; iHit++) {
    STHit *hit = new STHit(event -> GetHit(iHit));
    fHitQueue -> push_back(hit);
  }

  /////////////////////////////////////////////////////////////////////////
  
  std::sort(fHitQueue -> begin(), fHitQueue -> end(), STHitSortChargeInv());
  std::sort(fHitQueue -> begin(), fHitQueue -> end(), STHitSortZInv());

  BuildTracksFromQueue(1);
  SelectTracks(fTrackQueue, fTrackQueue);
  MergeTracks(fTrackQueue);

  BuildTracksFromQueue(0, -1, kTRUE);
  SelectTracks(fTrackQueue, fTrackQueue);

  /////////////////////////////////////////////////////////////////////////

  std::sort(fHitQueue -> begin(), fHitQueue -> end(), STHitSortZInv());
  std::sort(fHitQueue -> begin(), fHitQueue -> end(), STHitSortXInv());

  BuildTracksFromQueue(1);
  SelectTracks(fTrackQueue, fTrackQueue);
  MergeTracks(fTrackQueue);
  SelectTracks(fTrackQueue, fTrackBuffer);

  /////////////////////////////////////////////////////////////////////////

  std::sort(fHitQueue -> begin(), fHitQueue -> end(), STHitSortChargeInv());
  std::sort(fHitQueue -> begin(), fHitQueue -> end(), STHitSortYInv());

  fProxXCut = 1.01 * fXUnit;
  fProxYCut = 5 * fYUnit;
  fProxZCut = 1.01 * fZUnit;
  BuildTracksFromQueue(1);
  SelectTracks(fTrackQueue, fTrackQueue);

  fProxYCut = 12 * fYUnit;
  BuildTracksFromQueue(1);
  SelectTracks(fTrackQueue, fTrackQueue);
  MergeTracks(fTrackQueue);

  /////////////////////////////////////////////////////////////////////////

  SelectTracks(fTrackQueue, fTrackBuffer, 0);
  SortHits(fTrackBuffer);
}

void 
STLinearTrackFinder2::BuildTracksFromQueue(Int_t numIter, Int_t idxHitLimit, Bool_t emptyQueue)
{
  Int_t numHits = fHitQueue -> size();
  if (numHits == 0)
    return;

  for (Int_t iHit = 0; iHit < numHits; iHit++) 
  {
    Int_t idxHit = numHits - 1 - iHit;
    if (idxHit <= idxHitLimit)
      break;

    STHit *hit = fHitQueue -> at(idxHit); ////////

    STCoStatus statusFinal = kFAIL;

    Int_t idxMergeTrack = -1;
    Double_t rmsMin = 2 * fRMSTrackCut;

#ifdef DEBUG_cout_build
    std::cout << std::endl;
#endif

    Int_t numTracks = fTrackQueue -> size();
    for(Int_t iTrack = 0; iTrack < numTracks; iTrack++) 
    {
      STLinearTrack* track = fTrackQueue -> at(iTrack); /////////

#ifdef DEBUG_cout_build
      std::cout << "[BUILDER-" << numIter << "] Hit-" << idxHit << "/" << numHits
        << " Track-" << iTrack  << "(" << track -> GetNumHits() << ")/" 
        << numTracks << std::endl;
#endif
      if (emptyQueue == kTRUE)
        SetCutStretch(3, 1);

      Double_t rms = fRMSTrackCut;
      STCoStatus status = CorrelateHT(track, hit, rms); ///////////

      if (emptyQueue == kTRUE && status == kQUEUE) {
        status = kFAIL;
        SetCutStretch(1, 1);
      }

      if (status == kCANDIDATE) 
      {
        statusFinal = kCANDIDATE;
        if (rms < rmsMin) {
          rmsMin = rms;
          idxMergeTrack = iTrack;
        }
      }
      else if (status == kQUEUE && statusFinal != kCANDIDATE) 
        statusFinal = kQUEUE;
      //else if (status == kFAIL) // FINAL STATUS DO NOT CHANGE
        //continue;
#ifdef DEBUG_cout_status
      std::cout << " ++ Current status: " << status << std::endl;
      std::cout << " ++ Final   status: " << statusFinal << std::endl;
#endif
    }
#ifdef DEBUG_cout_build
    std::cout << "END Hit-" << idxHit << std::endl;
#endif

    if (statusFinal == kQUEUE && numIter != 0) 
    {
#ifdef DEBUG_cout_status
      std::cout << " !! \033[1;36m\033[40mBACK TO THE QUEUE\033[0m" << std::endl;
#endif
      continue;
    }

    if (statusFinal == kCANDIDATE) 
    {
#ifdef DEBUG_cout_status
      std::cout << " !! \033[1;32m\033[40mMERGE >> TRACK-" << idxMergeTrack 
                << "\033[0m" << std::endl;
#endif
      STLinearTrack* track = fTrackQueue -> at(idxMergeTrack);
      track -> AddHit(hit);
      fLTFitter -> FitAndSetTrack(track);
      fHitQueue -> erase(fHitQueue -> begin() + idxHit);
    }
    else if (iHit !=0 && iHit < fHitQueue -> size() - 1) {
#ifdef DEBUG_cout_status
      std::cout << " !! \033[0;31mTIME MACHINE!\033[0m" << std::endl;
#endif
      BuildTracksFromQueue(0, idxHit, kTRUE);
      iHit++;
    }
    else // if 1) statusFinal == kFAIL, 2) statusFinal == kQUEUE && numIter == 0
    {
      NewTrack(hit);
      fHitQueue -> erase(fHitQueue -> begin() + idxHit);
    }
  }

  if (numIter == 0)
    return;

  BuildTracksFromQueue(--numIter);
}

void
STLinearTrackFinder2::MergeTracks(std::vector<STLinearTrack*> *trackBuffer)
{
  Int_t numTracks = trackBuffer -> size();
  for(Int_t iTrack = 0; iTrack < numTracks; iTrack++) 
  {
    Int_t iIdxTrack = numTracks - 1 - iTrack;
    STLinearTrack* tracki = trackBuffer -> at(iIdxTrack); ////////

    STCoStatus statusFinal = kFAIL;

    Int_t idxMergeTrack = -1;
    Double_t rmsMin = fRMSTrackCut;

    Int_t numCompareTracks = iIdxTrack;
    for (Int_t jTrack = 0; jTrack < numCompareTracks; jTrack++)
    {
      Int_t jIdxTrack = numCompareTracks - 1 - jTrack;
      STLinearTrack* trackj = trackBuffer -> at(jIdxTrack); ////////

#ifdef DEBUG_cout_merge
      std::cout << std::endl;
      std::cout << "[MERGER] Track-" << iIdxTrack
                << "(" << tracki -> GetNumHits() << ")"
                << " Track-" << jIdxTrack
                << "(" << trackj -> GetNumHits() << ")" << std::endl;
#endif

      Double_t rms;
      STCoStatus status = CorrelateTT(tracki, trackj, rms);

      if (status == kCANDIDATE) 
      {
        statusFinal = kCANDIDATE;
        if (rms < rmsMin) {
          rmsMin = rms;
          idxMergeTrack = jIdxTrack;
        }
      }
    }

    if (statusFinal == kCANDIDATE)
    {
      // be careful when using erase of vector
      // merge tracki INTO trackj
      // add tracki to trackj and erase tracki 
      fLTFitter -> MergeAndSetTrack(trackBuffer -> at(idxMergeTrack), tracki);
      trackBuffer -> erase(trackBuffer -> begin() + iIdxTrack);
      break;
    }
  }
}


void 
STLinearTrackFinder2::SortHits(std::vector<STLinearTrack*> *trackBuffer)
{
  Int_t numTracks = trackBuffer -> size();
  for(Int_t iTrack = 0; iTrack < numTracks; iTrack++) 
  {
    STLinearTrack* track = trackBuffer -> at(iTrack);
    fLTFitter -> SortHits(track);
  }
}

void
STLinearTrackFinder2::SelectTracks(std::vector<STLinearTrack*> *trackBufferIn,
                                  std::vector<STLinearTrack*> *trackBufferOut,
                                  Int_t numHitsCut)
{
  if (numHitsCut == -1)
    numHitsCut = fNumHitsTrackCut;

  fHitQueue -> clear();

  Int_t numTracks = trackBufferIn -> size();
  for(Int_t iTrack = 0; iTrack < numTracks; iTrack++) 
  {
    Int_t idxTrack = numTracks - 1 - iTrack;
    STLinearTrack* track = trackBufferIn -> at(idxTrack);

    Int_t numHitsInTrack = track -> GetNumHits();

#ifdef DEBUG_cout_choose
    std::cout << "[SELECTOR] Track-" << iTrack 
              << "(" << numHitsInTrack << ") ";
#endif
#ifdef DEBUG_logger
    STDebugLogger::Instance() -> FillHist1("numHits", numHitsInTrack, 200, 0, 200);
#endif

    if (numHitsInTrack < numHitsCut) {
      for (Int_t iHit = 0; iHit < numHitsInTrack; iHit++)
        fHitQueue -> push_back(track -> GetHit(iHit));
#ifdef DEBUG_cout_choose
      std::cout << "BAD!" << std::endl;
#endif
    }
    else {
#ifdef DEBUG_cout_choose
    std::cout << "GOOD!" << std::endl;
#endif
      trackBufferOut -> push_back(track);
    }

    trackBufferIn -> erase(trackBufferIn -> begin() + idxTrack);
  }
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::CorrelateHT(STLinearTrack* track, STHit* hit, Double_t &rms)
{
  STCoStatus status = kCANDIDATE;

  status = CorrelateHTProximity(track, hit);
  if (status != kCANDIDATE) 
    return status;

  status = CorrelateHTPerpPlane(track, hit);
  if (status != kCANDIDATE) 
    return status;

  status = CorrelateHTPerpLine(track, hit);
  if (status != kCANDIDATE) 
    return status;

  status = RMSTestHT(track, hit, rms);

  return status;
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::CorrelateTT(STLinearTrack* track1, STLinearTrack* track2, Double_t &rms)
{
  STCoStatus status = kCANDIDATE;

  status = CorrelateTTPerp(track1, track2);
  if (status != kCANDIDATE) 
    return status;

  status = CorrelateTTGeo(track1, track2);
  if (status != kCANDIDATE) 
    return status;

  status = RMSTestTT(track1, track2, rms);

  return status;
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::CorrelateHTProximity(STLinearTrack* track, STHit* hit)
{
  Int_t numHitsInTrack = track -> GetNumHits();
  Int_t nHitIter = numHitsInTrack;

  STCoStatus statusFail = kQUEUE;
  Int_t scaling = 1;

  if (nHitIter >= fNumHitsCompare) {
    nHitIter = fNumHitsCompare;
  }
  else {
    scaling = 2;
    statusFail = kQUEUE;
  }


  TVector3 posH = hit -> GetPosition();

  for (Int_t iHit = 0; iHit < nHitIter; iHit++)
  {
    STHit *hitT = track -> GetHit(numHitsInTrack - 1 - iHit);
    TVector3 posT = hitT -> GetPosition();

    Double_t dX = posT.X()-posH.X();
    Double_t dZ = posT.Z()-posH.Z();
    Double_t dY = posT.Y()-posH.Y();

    if (dX < 0) dX *= -1;
    if (dY < 0) dY *= -1;
    if (dZ < 0) dZ *= -1;

    Double_t dR = TMath::Sqrt(dX*dX + dZ*dZ);

#ifdef DEBUG_cout_position
    std::cout << " @@ positionH: " 
              << posH.X() << " " << posH.Y() << " " << posH.Z() << std::endl;
    std::cout << "    positionT: "
              << posT.X() << " " << posT.Y() << " " << posT.Z() << std::endl;
#endif
#ifdef DEBUG_cout_build
    std::cout << " =" << iHit << " proximity: "
              << dX << " " << dY << " " << dZ << " " << dR << " / " 
              << fProxXCut << " "
              << fProxYCut << " "
              << fProxZCut << " "
              << scaling * fProxRCut << "(" << fProxRCut << ")" << std::endl;
#endif 
#ifdef DEBUG_logger
    STDebugLogger::Instance() -> FillHist1("dX",dX,200,0,50);
    STDebugLogger::Instance() -> FillHist1("dY",dY,200,0,50);
    STDebugLogger::Instance() -> FillHist1("dZ",dZ,200,0,50);
    STDebugLogger::Instance() -> FillHist1("dR",dR,200,0,50);
#endif

    //if (dY <= fStretchY * fProxYCut)
    Double_t proxYCutTemp = fStretchY * fYUnit *  fNumTbs / (hit -> GetLayer());
    if (dY <= proxYCutTemp)
    {
      if (dX <= fStretchXZ * fProxXCut && dZ <= fStretchXZ * fProxZCut)
        return kCANDIDATE;
      if (dR <= scaling * fProxRCut)
        return statusFail;
    }
  }

  return kFAIL;
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::CorrelateHTPerpPlane(STLinearTrack* track, STHit* hit)
{
  Int_t numHitsInTrack = track -> GetNumHits();
  if (numHitsInTrack <= fNumHitsFit) 
    return kCANDIDATE;

  Double_t scaling = 1.;
  if (numHitsInTrack < fNumHitsTrackCut) 
    scaling += 0.5 * (1 - (Double_t)numHitsInTrack/fNumHitsTrackCut);

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t perpPlane = (fLTFitter -> PerpPlane(track, hit)).Mag();

#ifdef DEBUG_cout_build
  std::cout << " == perpPlane: " << perpPlane 
            << " / " << scaling * fRMSPlaneCut 
            << "(" << fRMSPlaneCut << ")" << std::endl;
#endif
#ifdef DEBUG_logger
  STDebugLogger::Instance() -> FillHist1("perpPlane",perpPlane,200,0,50);
#endif

  if (perpPlane < scaling * fRMSPlaneCut) 
    return kCANDIDATE;
  else 
    return kFAIL;
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::CorrelateHTPerpLine(STLinearTrack* track, STHit* hit)
{
  Int_t numHitsInTrack = track -> GetNumHits();
  if (numHitsInTrack <= fNumHitsFit) 
    return kCANDIDATE;

  Double_t scaling = 1.;
  if (numHitsInTrack < fNumHitsTrackCut)
    scaling += 1 - (Double_t)numHitsInTrack/fNumHitsTrackCut;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t perpLine = (fLTFitter -> PerpLine(track, hit)).Mag();

#ifdef DEBUG_cout_build
  std::cout << " == perpLine : " << perpLine 
            << " / " << scaling * fRMSLineCut 
            << "(" << fRMSLineCut << ")" << std::endl;
#endif
#ifdef DEBUG_logger
  STDebugLogger::Instance() -> FillHist1("perpLine",perpLine,200,0,50);
  STDebugLogger::Instance() -> FillHist2("perpL_charge",
                                         perpLine, hit->GetCharge(),
                                         200,0,20, 200,0,500);
#endif

  if (perpLine < scaling * fRMSLineCut) 
    return kCANDIDATE;
  else 
    return kFAIL;
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::RMSTestHT(STLinearTrack* track, STHit* hit, Double_t &rms)
{
  Int_t numHitsInTrack = track -> GetNumHits();
  if (numHitsInTrack <= fNumHitsFit)
    return kCANDIDATE;

  Double_t scaling = 1.;
  if (numHitsInTrack < fNumHitsTrackCut)
    scaling += 1 - (Double_t)numHitsInTrack/fNumHitsTrackCut;

  if (track -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track);

  Double_t rmsL, rmsP;
  fLTFitter -> Fit(track, hit, rmsL, rmsP);
#ifdef DEBUG_cout_build
  std::cout << " == rms      : " << rmsL << " " << rmsP << " / "
            << scaling * fRMSLineCut  << "(" << fRMSLineCut << ") "
            << scaling * fRMSPlaneCut << "(" << fRMSPlaneCut << ") " << std::endl;
#endif
#ifdef DEBUG_logger
  STDebugLogger::Instance() -> FillHist1("rmsL",rmsL,500,0,10);
  STDebugLogger::Instance() -> FillHist1("rmsP",rmsP,500,0,10);
#endif

  rms = TMath::Sqrt(rmsL*rmsL + rmsP*rmsP);

  if (rmsL < scaling * fRMSLineCut && rmsP < scaling * fRMSPlaneCut)
    return kCANDIDATE;
  else 
    return kFAIL;
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::CorrelateTTGeo(STLinearTrack* track1, STLinearTrack* track2)
{
  Int_t numHitsInTracks = track1 -> GetNumHits() + track2 -> GetNumHits();

  Double_t scaling = 1.;
  if (numHitsInTracks < fNumHitsTrackCut)
    scaling -= 0.1 * (1 - (Double_t)numHitsInTracks/fNumHitsTrackCut);

  Double_t dotLine  = track1 -> GetDirection().Dot(track2 -> GetDirection());
  Double_t dotPlane = track1 -> GetNormal().Dot(track2 -> GetNormal());

#ifdef DEBUG_cout_merge
  std::cout << " == dot: " << dotLine << " " << dotPlane << " / "
            << fDirectionDotCut << " " << fNormalDotCut << std::endl;
#endif
#ifdef DEBUG_logger
  STDebugLogger::Instance() -> FillHist1("dotL",dotLine,100,0,1);
  STDebugLogger::Instance() -> FillHist1("dotP",dotPlane,100,0,1);
#endif

  //if (dotLine  > scaling * fDirectionDotCut &&
      //dotPlane > scaling * fNormalDotCut)
  if (dotLine  > scaling * fDirectionDotCut)
    return kCANDIDATE;
  else 
    return kQUEUE;
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::CorrelateTTPerp(STLinearTrack* track1, STLinearTrack* track2)
{
  Int_t numHitsInTracks = track1 -> GetNumHits() + track2 -> GetNumHits();

  Double_t scaling = 1.;
  if (numHitsInTracks < fNumHitsTrackCut)
    scaling += 1 - (Double_t)numHitsInTracks/fNumHitsTrackCut;

  Double_t perpL1 = (fLTFitter -> PerpLine (track1, track2 -> GetCentroid())).Mag();
  Double_t perpL2 = (fLTFitter -> PerpLine (track2, track1 -> GetCentroid())).Mag();
  Double_t perpP1 = (fLTFitter -> PerpPlane(track1, track2 -> GetCentroid())).Mag();
  Double_t perpP2 = (fLTFitter -> PerpPlane(track2, track1 -> GetCentroid())).Mag();

#ifdef DEBUG_cout_merge
  std::cout <<" == perpLine-TT : " << perpL1 << ", " << perpL2 << " / " 
            << fRMSLineCut << std::endl;
  std::cout <<" == perpPlane-TT: " << perpP1 << ", " << perpP2 << " / " 
            << fRMSPlaneCut << std::endl;
#endif
#ifdef DEBUG_logger
  STDebugLogger::Instance() -> FillHist1("perpLineTT",perpL1,200,0,50);
  STDebugLogger::Instance() -> FillHist1("perpLineTT",perpL2,200,0,50);
  STDebugLogger::Instance() -> FillHist1("perpPlaneTT",perpP1,200,0,50);
  STDebugLogger::Instance() -> FillHist1("perpPlaneTT",perpP2,200,0,50);
#endif

  if (perpL1 < scaling * fRMSLineCut  &&
      perpL2 < scaling * fRMSLineCut  &&
      perpP1 < scaling * fRMSPlaneCut && 
      perpP2 < scaling * fRMSPlaneCut)
    return kCANDIDATE;
  else 
    return kQUEUE;
}

STLinearTrackFinder2::STCoStatus
STLinearTrackFinder2::RMSTestTT(STLinearTrack* track1, STLinearTrack* track2, Double_t &rms)
{
  Int_t numHitsInTracks = track1 -> GetNumHits() + track2 -> GetNumHits();

  Double_t scaling = 1.;
  if (numHitsInTracks < fNumHitsTrackCut)
    scaling += 1 - (Double_t)numHitsInTracks/fNumHitsTrackCut;

  Double_t rmsL, rmsP;
  fLTFitter -> Fit(track1, track2, rmsL, rmsP);

#ifdef DEBUG_cout_merge
  std::cout << " == rms-TT: " << rmsL << " " << rmsP << " / "
            << fRMSLineCut << " " << fRMSPlaneCut << std::endl;
#endif
#ifdef DEBUG_logger
  STDebugLogger::Instance() -> FillHist1("rmsL",rmsL,500,0,10);
  STDebugLogger::Instance() -> FillHist1("rmsP",rmsP,500,0,10);
#endif

  rms = TMath::Sqrt(rmsL*rmsL + rmsP*rmsP);

  if (rmsL < scaling * fRMSLineCut && rmsP < scaling * fRMSPlaneCut) 
    return kCANDIDATE;
  else
    return kQUEUE;
}

void 
STLinearTrackFinder2::NewTrack(STHit* hit)
{
#ifdef DEBUG_cout_status
  std::cout << " !! \033[0;31mCREATE NEW TRACK\033[0m" << std::endl;
#endif
  STLinearTrack *track = new STLinearTrack();
  track -> AddHit(hit);
  track -> SetTrackID(fTrackQueue -> size());
  fTrackQueue -> push_back(track);
}

void
STLinearTrackFinder2::SetCutStretch(Double_t stretchXZ, Double_t stretchY)
{
  fStretchXZ = stretchXZ;
  fStretchY = stretchY;
}
