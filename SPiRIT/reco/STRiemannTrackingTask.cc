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
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STRiemannTrackingTask.hh"
#include "STHitCluster.hh"
#include "STRiemannTrackFinder.hh"
#include "STRiemannHit.hh"
#include "STSystemManipulator.hh"

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

/*
#include "GFTrackCand.h"
#include "GFTrack.h"
#include "RKTrackRep.h"

#include "GFDetPlane.h"
#include "TDatabasePDG.h"
#include "PndConstField.h"
#include "PndMultiField.h"
#include "PndFieldAdaptor.h"
#include "GFFieldManager.h"
#include "PndTrackCand.h"
#include "PndTrack.h"
#include "PndMCTrack.h"
*/

// Class Member definitions -----------

#define MINHITS 2

ClassImp(STRiemannTrackingTask);

STRiemannTrackingTask::STRiemannTrackingTask()
:FairTask("ST Pattern Reconstruction")
{
  fLogger = FairLogger::GetLogger();

  fIsPersistence = kFALSE;
  fMaxRadius = 1446.;

  fSortingMode = kTRUE;
  fSorting = STRiemannSort::kSortR;
  fInteractionZ = 42.78;

  // SPiRIT Settings
  fMinPoints = 3;
  fProxCut = 1.9;
  fProxZStretch = 1.6;
  fHelixCut = 0.2;
  fMaxRMS = 0.15;

  fMergeTracks = kTRUE;
  fTTProxCut = 15.0;
  fTTDipCut = 0.2;
  fTTHelixCut = 0.5;
  fTTPlaneCut = 0.3;

  fRiemannScale = 86.1;

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

  fMergeCurlers = kTRUE;
  fBlowUp = 1.;

  fSkipCrossingAreas = kTRUE;

  fDoMultiStep = kTRUE;
  fMinHitsZ = 2;
  fMinHitsR = 2;
  fMinHitsPhi = 2;

  fCounter = 0;

  fVerbose = kFALSE;
}

STRiemannTrackingTask::~STRiemannTrackingTask()
{
}

// Simple setter methods ----------------------------------------------------------------------------------------------------------
void STRiemannTrackingTask::SetVerbose(Bool_t value) { fVerbose = value; }
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
// --------------------------------------------------------------------------------------------------------------------------------

InitStatus
STRiemannTrackingTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();

  if(ioMan == 0){
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find FairRootManager!");

    return kERROR;
  }
  
  /*fMvdArray = (TClonesArray *) ioman -> GetObject("MVDPoint");
    if(fMvdArray == 0) {
    Error("STRiemannTrackingTask::Init", "mvd-array not found!");
    }
  */

  fEventHCArray = (TClonesArray *) ioMan -> GetObject("STEventHC");
  if (fEventHCArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEventHC array!");

    return kERROR;
  }

  fRiemannTrackArray = new TClonesArray("STRiemannTrack");
  ioMan -> Register("STRiemannTrack", "SPiRIT", fRiemannTrackArray, fIsPersistence);

  fRiemannHitArray = new TClonesArray("STRiemannHit");
  ioMan -> Register("STRiemannHit", "SPiRIT", fRiemannHitArray, fIsPersistence);

  fEventHCMArray = new TClonesArray("STEvent");
  ioMan -> Register("STEventHCM", "SPiRIT", fEventHCMArray, fIsPersistence);

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
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}



void
STRiemannTrackingTask::Exec(Option_t *opt)
{
//  if (fVerbose)
//    fLogger -> Info(MESSAGE_ORIGIN, "Event Number?");

  // Reset output Arrays
  if (fRiemannTrackArray == 0)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find RiemannTrackArray!");
  fRiemannTrackArray -> Delete();

  if (fRiemannHitArray == 0)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find RiemannHitArray!");
  fRiemannHitArray -> Delete();

  STEvent *eventHC = (STEvent *) fEventHCArray -> At(0);

  STSystemManipulator manipulator = STSystemManipulator();
  new ((*fEventHCMArray)[0]) STEvent(manipulator.Manipulate(eventHC));
  STEvent *eventHCM = (STEvent *) fEventHCMArray -> At(0);

  // clean up fRiemannList!
  for (Int_t i = 0; i < fRiemannList.size(); ++i){
    if (fRiemannList[i] != NULL) {
      fRiemannList[i] -> DeleteHits();

      delete fRiemannList[i];
    }
  }
  fRiemannList.clear();

  fClusterBuffer -> clear();

  if (fVerbose)
    std::cout << "Fetching clusters from cluster branch..." << std::endl;

  UInt_t numCluster = eventHCM -> GetNumClusters();
  for(UInt_t iCluster = 0; iCluster < numCluster; iCluster++){
    STHitCluster *cluster = (STHitCluster *) eventHCM -> GetCluster(iCluster);
    fClusterBuffer -> push_back(cluster);
  }

  if (fVerbose)
    std::cout << "Starting Pattern Reco..." << std::endl;

  std::vector<STRiemannTrack *> riemannTemp; // temporary storage, reused for every sector

  // first loop over sectors
  if (fClusterBuffer -> size() == 0)
    return;

  if (fVerbose)
    std::cout << "\n... building tracks from " << fClusterBuffer -> size() << " clusters" << std::endl;

  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortZ, fMinHitsZ, 0.7*fMaxRMS);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortR, fMinHitsR, fMaxRMS);

  riemannTemp.clear();

  if(fMergeTracks) {
    if (fVerbose)
      std::cerr << "\n merge of fRiemannList: merge " << fRiemannList.size() << " tracks ... ";
      
    fTrackFinder -> MergeTracks(fRiemannList);
    if (fVerbose)
      std::cerr << " done - created " << fRiemannList.size() << " merged tracks" << std::endl;
  }

  if (fClusterBuffer -> size() == 0)
    return;

  if (fVerbose)
    std::cerr << "\n... building tracks from " << fClusterBuffer -> size() << " clusters" << std::endl;

  // fill riemannTemp with tracks lying in the sector
  for (UInt_t iTrack = 0; iTrack < fRiemannList.size(); iTrack++)
    riemannTemp.push_back(fRiemannList[iTrack]);

  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortPhi, fMinHitsPhi, fMaxRMS);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortReversePhi, fMinHitsPhi, fMaxRMS);

  riemannTemp.clear();

  if(fMergeTracks) {
    if (fVerbose)
      std::cerr << "\n merge of fRiemannList: merge " << fRiemannList.size() << " tracks ... ";

    fTrackFinder -> MergeTracks(fRiemannList);

    if (fVerbose)
      std::cerr << " done - created " << fRiemannList.size() << " merged tracks" <<std::endl;
  }

  if (fClusterBuffer -> size() == 0)
    return;

  if (fVerbose)
    std::cerr << "\n... building tracks from " << fClusterBuffer -> size() << " clusters" << std::endl;

  for (UInt_t iTrack = 0; iTrack < fRiemannList.size(); iTrack++)
    riemannTemp.push_back(fRiemannList[iTrack]);

  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortZ, fMinPoints+1, fMaxRMS*1.5);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortR, fMinPoints+3, fMaxRMS);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortPhi, fMinPoints+1, fMaxRMS*1.5);
  BuildTracks(fTrackFinder, fClusterBuffer, &riemannTemp, STRiemannSort::kSortReversePhi, fMinPoints+1, fMaxRMS*1.5);

  riemannTemp.clear();

  if(fMergeTracks) {
    if (fVerbose)
      std::cerr << "\nfinal merge of fRiemannList: merge " << fRiemannList.size() << " tracks ... ";

    fTrackFinder -> MergeTracks(fRiemannList);
    if (fVerbose)
      std::cerr << " done - created " << fRiemannList.size() << " merged tracks" <<std::endl;
  }

  if(fMergeCurlers) {
    std::vector<STRiemannTrack *> riemannTempCurl;
    for (UInt_t iTrack = 0; iTrack < fRiemannList.size(); iTrack++) {
      if (fRiemannList[iTrack] -> IsFitted() && fRiemannList[iTrack] -> GetNumHits() > 5
          && fRiemannList[iTrack] -> GetR() < 30. && fabs(fRiemannList[iTrack] -> GetM()*1.57) < 120) { // Pi/2
        riemannTempCurl.push_back(fRiemannList[iTrack]);
        fRiemannList.erase(fRiemannList.begin() + iTrack);
        iTrack--;
      }
    }

    if (fVerbose)
      std::cerr << "\nmerge curlers: merge " << riemannTempCurl.size() << " tracks ... ";

    fTrackFinderCurl -> MergeTracks(riemannTempCurl);

    if (fVerbose)
      std::cerr << " done - created " << riemannTempCurl.size() << " merged tracks" <<std::endl;

    for (UInt_t iTrack = 0; iTrack < riemannTempCurl.size(); iTrack++)
      fRiemannList.push_back(riemannTempCurl[iTrack]);
  }// end merge curlers

  // clear small tracklets with < MINHITS hits
  for (UInt_t iTrack = 0; iTrack < fRiemannList.size(); iTrack++) {
    if (fRiemannList[iTrack] -> GetNumHits() < MINHITS){
      fRiemannList.erase(fRiemannList.begin() + iTrack);
      iTrack++;
    }
  }

  UInt_t foundTracks = fRiemannList.size();

  // store STRiemannTracks and Hits in output array
  STRiemannTrack *track;
  UInt_t numUsedCluster = 0, numHits;
  for (UInt_t iTrack = 0; iTrack < foundTracks; iTrack++){
    track = fRiemannList[iTrack];
    numHits = track -> GetNumHits();
    numUsedCluster += numHits;

    new((*fRiemannTrackArray)[fRiemannTrackArray -> GetEntriesFast()]) STRiemannTrack(*track);
    for(UInt_t iHit = 0; iHit < numHits; iHit++){
      STRiemannHit *hit = track -> GetHit(iHit);
      new ((*fRiemannHitArray)[fRiemannHitArray -> GetEntriesFast()]) STRiemannHit(*hit);
    }
  }

  if (fVerbose) {
    std::cerr << "Pattern Reco finished, found tracks: " << foundTracks << "\n";
    std::cerr << "used  " << numUsedCluster << " of " << numCluster << " Clusters \n";

    if (foundTracks != 0 || numUsedCluster != 0)
      fLogger -> Info(MESSAGE_ORIGIN, Form("FoundTracks: %d and usedCluster: %d", foundTracks, numUsedCluster));
  }
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

  trackfinder -> SetSorting(sorting);
  trackfinder -> SetMinHits(minHits);
  trackfinder -> SkipCrossingAreas(skipCrossingAreas);
  trackfinder -> SetSkipAndDelete(skipAndDelete);

  std::vector<STRiemannTrack *> trackletListCopy = *trackletList;

  Int_t nTracksIn = trackletList -> size();
  Int_t nClIn = clusterBuffer -> size();


  STRiemannTrack *LastTrackIn;
  if(nTracksIn > 0)
    LastTrackIn = trackletList -> back();

  trackfinder -> BuildTracks(*clusterBuffer, *trackletList);

  UInt_t nGoodTrks = 0, nErasedCl = 0, nHits;
  STRiemannTrack *trk;

  for(UInt_t i = 0; i < trackletList -> size(); ++i){
    trk = (*trackletList)[i];

    nHits = trk -> GetNumHits();
    std::cout << "   " << nHits << " hits in tracklet." << std::endl;

    if (trk -> DistRMS() < maxRMS && (nHits >= minHits || trk -> IsGood())) {
      trk -> SetFinished(kFALSE);
      trk -> SetGood();
      // clear clusters from good tracklets
      for(UInt_t iCl = 0; iCl < nHits; ++iCl){
        clusterBuffer -> erase(remove(clusterBuffer -> begin(), clusterBuffer -> end(), trk -> GetHit(iCl) -> GetCluster()),
        clusterBuffer -> end() );
      }
      ++nGoodTrks;

      //push back unique track to riemannlist
      if (std::find(trackletListCopy.begin(), trackletListCopy.end(), trk) == trackletListCopy.end()){
        fRiemannList.push_back(trk);
      }
    } else { // delete bad tracklets
      if (trk -> IsGood()) { // track has been found before ( -> clusters were taken out) but does not pass quality criteria anymore  ->  fill clusters back into buffer
        for (UInt_t iCl = 0; iCl < nHits; ++iCl) {
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

  if (fVerbose)
    std::cout << "   found good tracks: " << nGoodTrks - nTracksIn << ", reduced nCl by " << nClIn - clusterBuffer -> size() << std::endl;
}
