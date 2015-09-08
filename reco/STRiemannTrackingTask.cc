//-----------------------------------------------------------
// Description:
//      Implementation of class STRiemannTrackingTask
//      see STRiemannTrackingTask.hh for details
//
// Environment:
//      Software developed for the SPiRIT-TPC at RIBF-RIKEN
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//      Johannes Rauch
//      Felix Boehmer
//
// Redesigned by:
//      Genie Jhang          Korea University
//      JungWoo Lee          Korea University
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STRiemannTrackingTask.hh"
#include "STHitCluster.hh"
#include "STRiemannTrackFinder.hh"
#include "STRiemannHit.hh"
#include "STDebugLogger.hh"

#include "STProximityHTCorrelator.hh"
#include "STHelixHTCorrelator.hh"
#include "STDipTTCorrelator.hh"
#include "STRiemannTTCorrelator.hh"
#include "STDigiPar.hh"

#include "STEvent.hh"

// FairROOT classes
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairField.h"
#include "FairMCPoint.h"
#include "FairRunAna.h"

// STL
#include <map>
#include <cmath>

// ROOT classes
#include "TFile.h"
#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include "TDatabasePDG.h"


ClassImp(STRiemannTrackingTask);

STRiemannTrackingTask::STRiemannTrackingTask()
:FairTask("ST Pattern Reconstruction")
{
  fLogger = FairLogger::GetLogger();

  fIsPersistence = kFALSE;

  fSortingMode = kTRUE;
  fSorting = STRiemannSort::kSortR;
  fInteractionZ = 42.78;

  // SPiRIT Settings
  fMinPoints = 3;
  fProxCut = 40;
  fProxZStretch = 1.6; // 1.6;
  fHelixCut = 10;
  fMaxRMS = 5;

  fMergeTracks = kTRUE;
  fTTProxCut = 40;
  fTTDipCut = 50;
  fTTHelixCut = 40;
  fTTPlaneCut = 40;

  fRiemannScale = 25.0;

  fMergeCurlers = kTRUE;
  fBlowUp = 1.;

  fSkipCrossingAreas = kTRUE;

  fDoMultiStep = kTRUE;
  fMinHitsZ = 2;
  fMinHitsR = 2;
  fMinHitsPhi = 2;

  /*
  // PANDA settings
  fminpoints = 3;
  fproxcut = 1.9;
  fproxZstretch = 1.6;
  fhelixcut = 0.2;
  fmaxRMS = 0.15;

  fmergeTracks = kTRUE;
  fTTproxcut = 15.0;
  fTTdipcut = 0.2;
  fTThelixcut = 0.5;
  fTTplanecut = 0.3;

  friemannscale = 24.6;
  */

  /*
  // FOPI settings
  fminpoints = 3;
  fproxcut = 2.0;
  fproxZstretch = 1.6;
  fhelixcut = 0.4;
  fmaxRMS = 0.3;

  fmergeTracks = kTRUE;
  fTTproxcut = 15.0;
  fTTdipcut = 0.2;
  fTThelixcut = 0.5;
  fTTplanecut = 0.3;

  friemannscale = 8.6;
  */
}

STRiemannTrackingTask::~STRiemannTrackingTask()
{
}

// Simple setter methods ----------------------------------------------------------------------------------------------------------
void STRiemannTrackingTask::SetPersistence(Bool_t value) { fIsPersistence = value; }
void STRiemannTrackingTask::SetMaxRMS(Double_t value) { fMaxRMS = value; }
void STRiemannTrackingTask::SetMergeTracks(Bool_t mergeTracks) { fMergeTracks = mergeTracks; }
void STRiemannTrackingTask::SetRiemannScale(Double_t riemannScale) { fRiemannScale = riemannScale; }
void STRiemannTrackingTask::SkipCrossingAreas(Bool_t value) { fSkipCrossingAreas = value; }

void STRiemannTrackingTask::SetSortingParameters(Bool_t sortingMode, Int_t sorting, Double_t interactionZ)
{
  fSortingMode = sortingMode;
  fSorting = sorting;
  fInteractionZ = interactionZ;
}

void STRiemannTrackingTask::SetMultistepParameters(Bool_t doMultistep, UInt_t minHitsZ, UInt_t minHitsR, UInt_t minHitsPhi)
{
  fDoMultiStep = doMultistep;
  fMinHitsZ = minHitsZ;
  fMinHitsR = minHitsR;
  fMinHitsPhi = minHitsPhi;
}

void STRiemannTrackingTask::SetTrkFinderParameters(Double_t proxcut, Double_t helixcut, UInt_t minpointsforfit, Double_t zStretch)
{
  fProxCut = proxcut;
  fHelixCut = helixcut;
  fMinPoints = minpointsforfit;
  fProxZStretch = zStretch;
}

void STRiemannTrackingTask::SetTrkMergerParameters(Double_t TTproxcut, Double_t TTdipcut, Double_t TThelixcut, Double_t TTplanecut)
{
  fTTProxCut = TTproxcut;
  fTTDipCut = TTdipcut;
  fTTHelixCut = TThelixcut;
  fTTPlaneCut = TTplanecut;
}

void STRiemannTrackingTask::SetMergeCurlers(Bool_t mergeCurlers, Double_t blowUp)
{
  fMergeCurlers = mergeCurlers;
  fBlowUp = blowUp;
}

InitStatus
STRiemannTrackingTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();

  if (ioMan == 0){
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find FairRootManager!");

    return kERROR;
  }
  
  fEventHCMArray = (TClonesArray *) ioMan -> GetObject("STEventHCM");
  if (fEventHCMArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEventHCM array!");

    return kERROR;
  }

  fRiemannTrackArray = new TClonesArray("STRiemannTrack");
  ioMan -> Register("STRiemannTrack", "SPiRIT", fRiemannTrackArray, fIsPersistence);

  fRiemannHitArray = new TClonesArray("STRiemannHit");
  ioMan -> Register("STRiemannHit", "SPiRIT", fRiemannHitArray, 0);

  fTrackFinder = new STRiemannTrackFinder();
  fTrackFinder -> SetSorting(fSorting);
  fTrackFinder -> SetInteractionZ(fInteractionZ);
  fTrackFinder -> SetSortingMode(fSortingMode);
  fTrackFinder -> SetMinHitsForFit(fMinPoints);

  fTrackFinder -> InitTracks(kFALSE);
  fTrackFinder -> SkipCrossingAreas(fSkipCrossingAreas);
  fTrackFinder -> SetSkipAndDelete(kFALSE);

  fTrackFinder -> SetScale(fRiemannScale);

  fTrackFinder -> SetProxCut(fProxCut);
  fTrackFinder -> SetTTProxCut(fTTProxCut);

  // Hit-Track Correlators
  fTrackFinder -> AddHTCorrelator(new STProximityHTCorrelator(fProxCut, fProxZStretch, fHelixCut));
  fTrackFinder -> AddHTCorrelator(new STHelixHTCorrelator(fHelixCut));

  // Track-Track Correlators
  fTrackFinder -> AddTTCorrelator(new STDipTTCorrelator(fTTProxCut, fTTDipCut, fTTHelixCut));
  fTrackFinder -> AddTTCorrelator(new STRiemannTTCorrelator(fTTPlaneCut, fMinPoints));

  // for merging curling tracks with increased TT helixcut
  fTrackFinderCurl = new STRiemannTrackFinder();
  fTrackFinderCurl -> SetSorting(fSorting);
  fTrackFinderCurl -> SetSortingMode(fSortingMode);
  fTrackFinderCurl -> SetMinHitsForFit(fMinPoints);
  fTrackFinderCurl -> SetScale(fRiemannScale);
  fTrackFinderCurl -> SetMaxNumHitsForPR(fMinPoints);

  fTrackFinderCurl -> SetProxCut(fProxCut);
  fTrackFinderCurl -> SetTTProxCut(30.);

  // Track-Track Correlators
  fTrackFinderCurl -> AddTTCorrelator(new STDipTTCorrelator(30., fBlowUp*fTTDipCut, fBlowUp*fTTHelixCut));
  fTrackFinderCurl -> AddTTCorrelator(new STRiemannTTCorrelator(1.5*fTTPlaneCut, 20));

  fClusterBuffer = new std::vector<STHitCluster *>;

  return kSUCCESS;
}


void
STRiemannTrackingTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run) fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db) fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar) fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}



void
STRiemannTrackingTask::Exec(Option_t *opt)
{
  // Clear buffers ----------------------------------------------------------------------------------------------------
  if (fRiemannTrackArray == 0)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find RiemannTrackArray!");
  fRiemannTrackArray -> Delete();

  if (fRiemannHitArray == 0)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find RiemannHitArray!");
  fRiemannHitArray -> Delete();

  if (fEventHCMArray -> GetEntriesFast() == 0)
    return;

  for (Int_t i = 0; i < fRiemannList.size(); ++i){
    if (fRiemannList[i] != NULL) {
      fRiemannList[i] -> DeleteHits();
      delete fRiemannList[i];
    }
  }
  fRiemannList.clear();
  fClusterBuffer -> clear();



  // Initialize -------------------------------------------------------------------------------------------------------
  STEvent *eventHCM = (STEvent *) fEventHCMArray -> At(0);
  fLogger -> Debug(MESSAGE_ORIGIN, "Fetching clusters from cluster branch...");

  UInt_t numCluster = eventHCM -> GetNumClusters();
  if (numCluster == 0)
    fLogger -> Info(MESSAGE_ORIGIN, "Event #%d : Bad event. No clusters to build tracks.");

  for (UInt_t iCluster = 0; iCluster < numCluster; iCluster++) {
    STHitCluster *cluster = (STHitCluster *) eventHCM -> GetCluster(iCluster);
    fClusterBuffer -> push_back(cluster);
  }

  std::vector<STRiemannTrack *> riemannTemp; // temporary storage
  fLogger -> Debug(MESSAGE_ORIGIN, "Starting Pattern Reco...");



  // 1st Build & Merge ------------------------------------------------------------------------------------------------
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortR, fMinHitsR, fMaxRMS);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortZ, fMinHitsZ, 0.7*fMaxRMS);
  riemannTemp.clear();

  MergeTracks();



  // 2nd Build & Merge ------------------------------------------------------------------------------------------------
  UInt_t nTracks = fRiemannList.size();
  for (UInt_t iTrack = 0; iTrack < nTracks; iTrack++)
    riemannTemp.push_back(fRiemannList[iTrack]);

  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortPhi, fMinHitsPhi, fMaxRMS);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortReversePhi, fMinHitsPhi, fMaxRMS);
  riemannTemp.clear();

  MergeTracks();



  // 3rd Build & Merge ------------------------------------------------------------------------------------------------
  nTracks = fRiemannList.size();
  for (UInt_t iTrack = 0; iTrack < nTracks; iTrack++)
    riemannTemp.push_back(fRiemannList[iTrack]);

  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortR, fMinPoints+3, fMaxRMS);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortZ, fMinPoints+1, fMaxRMS*1.5);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortPhi, fMinPoints+1, fMaxRMS*1.5);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortReversePhi, fMinPoints+1, fMaxRMS*1.5);
  riemannTemp.clear();

  MergeTracks();



  // Merge Curlers ----------------------------------------------------------------------------------------------------
  MergeCurlers();




  // Clear small tracklets with < 2 hits ------------------------------------------------------------------------------
  for (UInt_t iTrack = 0; iTrack < fRiemannList.size(); iTrack++) {
    if (fRiemannList[iTrack] -> GetNumHits() < 2){
      fRiemannList.erase(fRiemannList.begin() + iTrack);
      iTrack++;
    }
  }



  // Store ------------------------------------------------------------------------------------------------------------
  STRiemannTrack *track;
  UInt_t foundTracks = fRiemannList.size();
  UInt_t numUsedCluster = 0;
  UInt_t numHits;

  for (UInt_t iTrack = 0; iTrack < foundTracks; iTrack++){
    track = fRiemannList[iTrack];
    numHits = track -> GetNumHits();
    numUsedCluster += numHits;

    new((*fRiemannTrackArray)[fRiemannTrackArray -> GetEntriesFast()]) STRiemannTrack(*track);
    for (UInt_t iHit = 0; iHit < numHits; iHit++){
      STRiemannHit *hit = track -> GetHit(iHit);
      new ((*fRiemannHitArray)[fRiemannHitArray -> GetEntriesFast()]) STRiemannHit(*hit);
    }
  }



  fLogger -> Info(MESSAGE_ORIGIN, Form("Event #%d : Found %d tracks, used %d(/%d) clusters.", 
                       eventHCM -> GetEventID(), foundTracks, numUsedCluster, numCluster));
}


void STRiemannTrackingTask::BuildTracks(STRiemannTrackFinder *trackfinder,
                                        std::vector<STHitCluster *> *clusterBuffer,
                                        std::vector<STRiemannTrack *> *trackletList,
                                        Int_t sorting,
                                        UInt_t minHits,
                                        Double_t maxRMS,
                                        Bool_t skipCrossingAreas,
                                        Bool_t skipAndDelete)
{
  Int_t nClIn = clusterBuffer -> size();
  if (nClIn == 0)
    return;

  Int_t nTracksIn = trackletList -> size();

  fLogger -> Debug(MESSAGE_ORIGIN, Form("... building tracks from %d clusters", clusterBuffer -> size()));

  trackfinder -> SetSorting(sorting);
  trackfinder -> SetMinHits(minHits);
  trackfinder -> SkipCrossingAreas(skipCrossingAreas);
  trackfinder -> SetSkipAndDelete(skipAndDelete);

  std::vector<STRiemannTrack *> trackletListCopy = *trackletList;

  STRiemannTrack *LastTrackIn;
  if (nTracksIn > 0)
    LastTrackIn = trackletList -> back();

  trackfinder -> BuildTracks(*clusterBuffer, *trackletList);

  UInt_t nGoodTrks = 0, nErasedCl = 0, nHits;
  STRiemannTrack *trk;

  for (UInt_t i = 0; i < trackletList -> size(); i++) 
  {
    trk = (*trackletList)[i];

    nHits = trk -> GetNumHits();
    fLogger -> Debug(MESSAGE_ORIGIN, Form("   %d hits in tracklet.", nHits));

    if (trk -> DistRMS() < maxRMS && (nHits >= minHits || trk -> IsGood())) 
    {
#ifdef DEBUGRIEMANNCUTS // STDebugLogger.hh
      STDebugLogger::Instance() -> FillHist1("rms", trk -> DistRMS(), 100, 0, 30);
#endif
      trk -> SetFinished(kFALSE);
      trk -> SetGood();

      // clear clusters from good tracklets
      for (UInt_t iCl = 0; iCl < nHits; iCl++)
        clusterBuffer -> erase(remove(clusterBuffer -> begin(), clusterBuffer -> end(), trk -> GetHit(iCl) -> GetCluster()), clusterBuffer -> end());
     
      nGoodTrks++;
      fLogger -> Debug(MESSAGE_ORIGIN, "================================================================================ good Track!");

      //push back unique track to riemannlist
      if (std::find(trackletListCopy.begin(), trackletListCopy.end(), trk) == trackletListCopy.end()) 
        fRiemannList.push_back(trk);

    } 
    else // delete bad tracklets 
    {
      if (trk -> IsGood()) {
      // track has been found before ( -> clusters were taken out) but does not pass quality criteria anymore  ->  fill clusters back into buffer
        for (UInt_t iCl = 0; iCl < nHits; iCl++) {
          clusterBuffer -> push_back(trk -> GetHit(iCl) -> GetCluster());
        }
      }

      // delete hits and track
      trk -> DeleteHits();
      delete trk;
      trackletList -> erase(trackletList -> begin() + i);

      // also has to be removed from fRiemannList
      fRiemannList.erase(remove(fRiemannList.begin(), fRiemannList.end(), trk), fRiemannList.end());
      i--;
    }
  }

  fLogger -> Debug(MESSAGE_ORIGIN, Form("   nGoodTrks: %d,  nTracksIn: %d,  nClIn: %d, clusterBufferSize: %d", nTracksIn, nClIn, clusterBuffer -> size()));
  fLogger -> Debug(MESSAGE_ORIGIN, Form("   found good tracks: %d, reuduced nCl by %d", nGoodTrks - nTracksIn, nClIn - clusterBuffer -> size()));
}

void STRiemannTrackingTask::MergeTracks()
{
  if (!fMergeTracks) 
   return;

  fLogger -> Debug(MESSAGE_ORIGIN, Form("merge of fRiemannList: merge %d tracks ...", fRiemannList.size()));

  fTrackFinder -> MergeTracks(fRiemannList);

  fLogger -> Debug(MESSAGE_ORIGIN, Form(" done - created %d merged tracks", fRiemannList.size()));
}

void STRiemannTrackingTask::MergeCurlers()
{
  if (!fMergeCurlers) 
    return;

  std::vector<STRiemannTrack *> riemannTempCurl;

  for (UInt_t iTrack = 0; iTrack < fRiemannList.size(); iTrack++) 
  {
    if (   fRiemannList[iTrack] -> IsFitted() 
        && fRiemannList[iTrack] -> GetNumHits() > 5
        && fRiemannList[iTrack] -> GetR() < 30. 
        && fabs(fRiemannList[iTrack] -> GetM()*1.57) < 120)  // Pi/2
    {
      riemannTempCurl.push_back(fRiemannList[iTrack]);
      fRiemannList.erase(fRiemannList.begin() + iTrack);
      iTrack--;
    }
  }

  fLogger -> Debug(MESSAGE_ORIGIN, Form("merge curlers: merge %d tracks ... ", riemannTempCurl.size()));

  fTrackFinderCurl -> MergeTracks(riemannTempCurl);

  fLogger -> Debug(MESSAGE_ORIGIN, Form(" done - created %d merged tracks", riemannTempCurl.size()));

  for (UInt_t iTrack = 0; iTrack < riemannTempCurl.size(); iTrack++)
    fRiemannList.push_back(riemannTempCurl[iTrack]);
}
