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

// This Class' Header ------------------
#include "STRiemannTrackingTask.hh"

// C/C++ Headers ----------------------
#include <map>
#include <cmath>

// FairROOT Headers
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairField.h"
#include "FairMCPoint.h"
#include "FairRunAna.h"
#include "McIdCollection.h"

// SPiRIT Headers --------
#include "STCluster.hh"
#include "STRiemannTrackFinder.hh"
#include "STRiemannHit.hh"

#include "STProximityHTCorrelator.hh"
#include "STHelixHTCorrelator.hh"
#include "STDipTTCorrelator.hh"
#include "STRiemannTTCorrelator.hh"
#include "STDigiPar.hh"
#include "STDigiMapper.hh"

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

// ROOT Headers
#include "TFile.h"
#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include "TDatabasePDG.h"

// Class Member definitions -----------

#define MINHITS 4
#define PDGDEFAULT 211

ClassImp(STRiemannTrackingTask)

STRiemannTrackingTask::STRiemannTrackingTask()
: FairTask("ST Pattern Reconstruction")
{
  fPersistence = kFALSE;
  fNSectors = 1;
  fMaxRadius = 100;

  fSortingMode = true;
  fSorting = 3;
  fInteractionZ = 42.78;

  // SPiRIT Settings
  fMinPoints = 3;
  fProxCut = 1.9;
  fProxZStretch = 1.6;
  fHelixCut = 0.2;
  fMaxRMS = 0.15;

  fMergeTracks = true;
  fTTProxCut = 15.0;
  fTTDipCut = 0.2;
  fTTHelixCut = 0.5;
  fTTPlaneCut = 0.3;

  fRiemannScale = 24.6;

  /*
  // PANDA settings
  fminpoints = 3;
  fproxcut = 1.9;
  fproxZstretch = 1.6;
  fhelixcut = 0.2;
  fmaxRMS = 0.15;

  fmergeTracks = true;
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

  fmergeTracks = true;
  fTTproxcut = 15.0;
  fTTdipcut = 0.2;
  fTThelixcut = 0.5;
  fTTplanecut = 0.3;

  friemannscale = 8.6;
  */

  fMergeCurlers = true;
  fBlowUp = 1.;

  fSkipCrossingAreas = true;

  fDoMultiStep = true;
  fMinHitsZ = 10;
  fMinHitsR = 10;
  fMinHitsPhi = 10;

  fClusterBranchName = "STClusterAligned";
  fRiemannTrackBranchName = "RiemannTrack";
  fRiemannHitBranchName = "RiemannHit";
  fCounter = 0;

  fVerbose = kFALSE;
}

STRiemannTrackingTask::~STRiemannTrackingTask()
{
}

void 
STRiemannTrackingTask::SetSortingParameters(Bool_t sortingMode,
                                            Int_t sorting,
                                            Double_t interactionZ)
{
  fSortingMode = sortingMode;
  fSorting = sorting;
  fInteractionZ = interactionZ;
}

void
STRiemannTrackingTask::SetMultistepParameters(Bool_t doMultistep,
                                              UInt_t minHitsZ,
                                              UInt_t minHitsR,
                                              UInt_t minHitsPhi)
{
  fDoMultiStep = doMultistep;
  fMinHitsZ = minHitsZ;
  fMinHitsR = minHitsR;
  fMinHitsPhi = minHitsPhi;
}

void
STRiemannTrackingTask::SetTrkFinderParameters(Double_t proxcut,
                                              Double_t helixcut,
                                              UInt_t minpointsforfit,
                                              Double_t zStretch)
{
  fProxCut = proxcut;
  fHelixCut = helixcut;
  fMinPoints = minpointsforfit;
  fProxZStretch = zStretch;
}

void
STRiemannTrackingTask::SetTrkMergerParameters(Double_t TTproxcut,
                                              Double_t TTdipcut,
                                              Double_t TThelixcut,
                                              Double_t TTplanecut)
{
  fTTProxCut = TTproxcut;
  fTTDipCut = TTdipcut;
  fTTHelixCut = TThelixcut;
  fTTPlaneCut = TTplanecut;
}

InitStatus
STRiemannTrackingTask::Init()
{
  //Get ROOT Manager
  FairRootManager *ioman = FairRootManager::Instance();

  if(ioman == 0){
    Error("STRiemannTrackingTask::Init", "RootManager not instantiated!");
    return kERROR;
  }

  // Get input collection
  fClusterArray = (TClonesArray *) ioman -> GetObject(fClusterBranchName);
  if(fClusterArray == 0){
    Error("STRiemannTrackingTask::Init", "Cluster-array not found!");
    return kERROR;
  }

  /*fMvdArray = (TClonesArray *) ioman -> GetObject("MVDPoint");
    if(fMvdArray == 0) {
    Error("STRiemannTrackingTask::Init", "mvd-array not found!");
    }
  */

  friemannTrackArray = new TClonesArray("STRiemannTrack");
  ioman -> Register(fRiemannTrackBranchName, "ST", fRiemannTrackArray, fPersistence);

  friemannHitArray = new TClonesArray("STRiemannHit");
  ioman -> Register(fRiemannHitBranchName, "ST", fRiemannHitArray, fPersistence);



  fTrackFinder = new STRiemannTrackFinder();
  fTrackFinder -> SetSorting(fSorting);
  fTrackFinder -> SetInteractionZ(fInteractionZ);
  fTrackFinder -> SetSortingMode(fSortingMode);
  fTrackFinder -> SetMinHitsForFit(fMinpoints);

  fTrackFinder -> initTracks(false);
  fTrackFinder -> SkipCrossingAreas(fSkipCrossingAreas);
  fTrackFinder -> SetSkipAndDelete(false);

  fTrackFinder -> SetScale(fRiemannScale);

  fTrackFinder -> SetProxcut(fProxcut);
  fTrackFinder -> SetTTProxcut(_TTproxcut);

  // Hit-Track Correlators
  fTrackFinder -> AddCorrelator(new STProximityHTCorrelator(fProxcut, fproxZstretch, fhelixcut));
  fTrackFinder -> AddCorrelator(new STHelixHTCorrelator(fHelixcut));

  // Track-Track Correlators
  fTrackFinder -> AddTTCorrelator(new STDipTTCorrelator(_TTproxcut, fTTdipcut, fTThelixcut));
  fTrackFinder -> AddTTCorrelator(new STRiemannTTCorrelator(_TTplanecut, fminpoints));

  // for merging curling tracks with increased TT helixcut
  fTrackFinderCurl = new STRiemannTrackFinder();
  fTrackFinderCurl -> SetSorting(fSorting);
  fTrackFinderCurl -> SetSortingMode(fSortingMode);
  fTrackFinderCurl -> SetMinHitsForFit(fMinpoints);
  fTrackFinderCurl -> SetScale(fRiemannscale);
  fTrackFinderCurl -> SetMaxNumHitsForPR(fMinpoints);

  fTrackFinderCurl -> SetProxcut(fProxcut);
  fTrackFinderCurl -> SetTTProxcut(30.);

  // Track-Track Correlators
  fTrackFinderCurl -> AddTTCorrelator(new STDipTTCorrelator(30., fBlowUp*_TTdipcut, fBlowUp*_TThelixcut));
  fTrackFinderCurl -> AddTTCorrelator(new STRiemannTTCorrelator(1.5*_TTplanecut, 20));

  // get the maximum radius
  fMaxRadius = fPar -> GetRMax();

  fnsectors= fPar -> GetPadPlane() -> GetNSectors();
  std::cerr << "Found " << fnsectors << " sectors in padplane; outer radius = " << fMaxRadius << std::endl;
  for(UInt_t  isect=0;isect<fnsectors;++isect){
    fBuffermap[isect]=new std::vector<STCluster *>;
  }

  fTpcOffset = STDigiMapper::GetInstance() -> windowMin();
  fTpcLength = STDigiMapper::GetInstance() -> windowMax() - fTpcOffset;

  std::cout << "TPC Offset: " << fTpcOffset << "  \tTPC Length: " << fTpcLength << std::endl;

  return kSUCCESS;
}


void
STRiemannTrackingTask::SetParContainers() {

  std::cout << "STClusterFinderTask::SetParContainers" << std::endl;
  std::cout.flush();

  // Get run and runtime database
  FairRun *run = FairRun::Instance();
  if (!run)
    Fatal("SetParContainers", "No analysis run");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    Fatal("SetParContainers", "No runtime database");

  // Get ST digitisation parameter container
  fPar = (STDigiPar *) db -> GetContainer("STDigiPar");
  if (!fPar)
    Fatal("SetParContainers", "STDigiPar not found");
}



  void
STRiemannTrackingTask::Exec(Option_t *opt)
{
  if (fVerbose)
    std::cout << "STRiemannTrackingTask::Exec; Event Number: " << counter++ << std::endl;

  // Reset output Arrays
  if (fRiemannTrackArray == 0)
    Fatal("STRiemannTrackingTask::Exec", "No RiemannTrackArray");
  friemannTrackArray -> Delete();

  if (fRiemannHitArray==0)
    Fatal("STRiemannTrackingTask::Exec", "No RiemannHitArray");
  friemannHitArray -> Delete();

  // clean up fRiemannList!
  for (Int_t i = 0; i < fRiemannList.size(); ++i){
    if (fRiemannList[i] != NULL) {
      fRiemannList[i] -> deleteHits();
      delete fRiemannList[i];
    }
  }
  fRiemannList.clear();

  for (UInt_t isect=0; isect < fNSectors; ++isect)
    fBuffermap[isect] -> clear();

  if (fVerbose)
    std::cout << "Fetching clusters from cluster branch..." << std::endl;

  UInt_t ncl = fClusterArray -> GetEntriesFast();
  for (UInt_t isect = 0; isect < fnsectors; ++isect)
    fBuffermap[isect] -> reserve(ncl/fnsectors + 2000);
  for(UInt_t i = 0; i < ncl; ++i){
    STCluster *cluster = (STCluster *) fClusterArray -> At(i);
    UInt_t sectorId = cluster -> sector();
    fBuffermap[sectorId] -> push_back(cluster);
  }

  if (fVerbose)
    std::cout << "Starting Pattern Reco..." << std::endl;

  UInt_t nTotCl = 0; // number of total clusters
  for(UInt_t isect = 0; isect < fNSectors; ++isect){
    nTotCl += fBuffermap[isect] -> size();
  }

  std::vector<STRiemannTrack *> riemannTempSec; // temporary storage, reused for every sector

  // first loop over sectors
  for(UInt_t isect = 0; isect < fnsectors; ++isect){
    if (fBuffermap[isect] -> size() == 0)
      continue;
    if (fVerbose)
      std::cout << "\n... building tracks in sector " << isect << " from " << fBuffermap[isect] -> size() << " clusters" << std::endl;

    fClusterBuffer = fBuffermap[isect];

    buildTracks(fTrackfinder, fClusterBuffer, &riemannTempSec, 2, fminHitsZ, 0.7*fMaxRMS);
    buildTracks(fTrackfinder, fClusterBuffer, &riemannTempSec, 3, fminHitsR, fmaxRMS);

    riemannTempSec.clear();
  } // end loop over sectors


  if(fMergeTracks) {
    if (fVerbose)
      std::cerr << "\n merge of fRiemannList: merge " << fRiemannList.size() << " tracks ... ";
      
    fTrackFinder -> mergeTracks(fRiemannList);
    if (fVerbose)
      std::cerr << " done - created " << fRiemannList.size() << " merged tracks" << std::endl;
  }


  // resectorize: 10 sectors, left and right half of chamber, each 5 sectors in z
  Int_t factor, zSlice, nSlices = 5;
  if (fnsectors > 2*nSlices){
    std::vector<STCluster *> remainingClusters;
    remainingClusters.reserve(200000);

    for(UInt_t isect = 0; isect < fnsectors; ++isect) {
      for(UInt_t iCl = 0; iCl < fBufferMap[isect] -> size(); ++iCl) {
        remainingClusters.push_back((*fBufferMap[isect])[iCl]);
      }
    }

    for(UInt_t isect = 0; isect < fnsectors; ++isect) {
      fBufferMap[isect] -> clear();
    }

    STCluster *Cl;
    for(UInt_t iCl = 0; iCl < remainingClusters.size(); ++iCl) {
      Cl = remainingClusters[iCl];
      if (Cl -> pos().X() < 0)
        factor = 0;
      else
        factor = 1;

      for (zSlice = 0; zSlice < lSlices; ++zSlice) {
        if (Cl -> pos().Z() < fTpcOffset + (zSlice+1)*fTpcLength/nSlices)
          break;
      }

      fBufferMap[factor + 2*zSlice] -> push_back(Cl);
    }
  }

  // second loop over sectors
  Double_t lowLim = fTpcOffset, upLim;
  for(UInt_t isect = 0; isect < fnsectors; ++isect){
    if (fBufferMap[isect] -> size() == 0)
      continue;

    if (fVerbose)
      std::cerr << "\n... building tracks in sector " << isect << " from " << fBufferMap[isect] -> size() << " clusters" << std::endl;

    fClusterBuffer = fBufferMap[isect];

    // fill riemannTempSec with tracks lying in the sector
    upLim = lowLim + fTpcLength/nSlices;
    for (UInt_t i = 0; i < fRiemannList.size(); ++i) {
      Double_t z = fRiemannList[i] -> GetFirstHit() -> z();
      if (z > lowLim - 2 && z < upLim + 2) {
        riemannTempSec.push_back(fRiemannList[i]);
        continue;
      }

      z = fRiemannList[i] -> GetLastHit() -> z();
      if (z > lowLim - 2 && z <upLim + 2){
        riemannTempSec.push_back(fRiemannList[i]);
        continue;
      }
    }
    lowLim = upLim;

    buildTracks(fTrackfinder, fClusterBuffer, &riemannTempSec, 5, fminHitsPhi, fmaxRMS);
    buildTracks(fTrackfinder, fClusterBuffer, &riemannTempSec, -5, fminHitsPhi, fmaxRMS);

    riemannTempSec.clear();

  } // end loop over sectors


  if(fMergeTracks) {
    if (fVerbose)
      std::cerr << "\n merge of fRiemannList: merge " << fRiemannList.size() << " tracks ... ";

    fTrackFinder -> mergeTracks(fRiemannList);
    if (fVerbose)
      std::cerr << " done - created " << fRiemannList.size() << " merged tracks" <<std::endl;
  }



  // third loop over sectors
  lowLim = fTpcOffset;
  for(UInt_t isect = 0; isect < fnsectors; ++isect){
    if (fBufferMap[isect] -> size() == 0)
      continue;

    if (fVerbose)
      std::cerr << "\n... building tracks in sector " << isect << " from " << fBufferMap[isect] -> size() << " clusters" << std::endl;

    fClusterBuffer = fBufferMap[isect];

    // fill riemannTempSec with tracks lying in the sector
    upLim = lowLim + fTpcLength/nSlices;
    for (UInt_t i = 0; i < fRiemannList.size(); ++i) {
      Double_t z = fRiemannList[i] -> GetFirstHit() -> cluster() -> pos().Z();
      if (z > lowLim - 1 && z < upLim + 1){
        riemannTempSec.push_back(fRiemannList[i]);
        continue;
      }

      z = fRiemannList[i] -> GetLastHit() -> cluster() -> pos().Z();
      if (z > lowLim - 1 && z < upLim + 1){
        riemannTempSec.push_back(fRiemannList[i]);
        continue;
      }
    }
    lowLim = upLim;

    UInt_t nTrks = riemannTempSec.size();


    buildTracks(fTrackfinder, fClusterBuffer, &riemannTempSec, 2, fminpoints+1, fmaxRMS*1.5);
    buildTracks(fTrackfinder, fClusterBuffer, &riemannTempSec, 3, fminpoints+3, fmaxRMS);
    buildTracks(fTrackfinder, fClusterBuffer, &riemannTempSec, 5, fminpoints+1, fmaxRMS*1.5);
    buildTracks(fTrackfinder, fClusterBuffer, &riemannTempSec, -5, fminpoints+1, fmaxRMS*1.5);

    riemannTempSec.clear();
  } // end loop over sectors


  if(fMergeTracks) {
    if (fVerbose)
      std::cerr << "\nfinal merge of fRiemannList: merge " << fRiemannList.size() << " tracks ... ";

    fTrackFinder -> mergeTracks(fRiemannList);
    if (fVerbose)
      std::cerr << " done - created " << fRiemannList.size() << " merged tracks" <<std::endl;
  }

  if(fMergeCurlers) {
    std::vector<STRiemannTrack *> riemannTempCurl;
    for (UInt_t i = 0; i < fRiemannList.size(); ++i){
      if (fRiemannList[i] -> isFitted() && fRiemannList[i] -> GetNumHits() > 5
          && fRiemannList[i] -> r() < 30. && fabs(fRiemannList[i] -> m()*1.57) < 120) { // Pi/2
        riemannTempCurl.push_back(fRiemannList[i]);
        fRiemannList.erase(fRiemannList.begin() + i);
        --i;
      }
    }

    if (fVerbose)
      std::cerr << "\nmerge curlers: merge " << riemannTempCurl.size() << " tracks ... ";

    fTrackFinderCurl -> mergeTracks(riemannTempCurl);
    if (fVerbose)
      std::cerr << " done - created " << riemannTempCurl.size() << " merged tracks" <<std::endl;

    for (UInt_t i = 0; i < riemannTempCurl.size(); ++i){
      fRiemannList.push_back(riemannTempCurl[i]);
    }
  }// end merge curlers

  // clear small tracklets with < MINHITS hits
  for (UInt_t i = 0; i < fRiemannList.size(); ++i){
    if (fRiemannlist[i] -> GetNumHits() < MINHITS){
      fRiemannlist.erase(fRiemannlist.begin() + i);
      --i;
    }
  }


  UInt_t foundTrks = fRiemannList.size();

  // store STRiemannTracks and Hits in output array
  STRiemannTrack *trk;
  UInt_t nUsedCl = 0, nHits;
  for (UInt_t i = 0; i < foundTrks; ++i){
    trk = fRiemannlist[i];
    nHits = trk -> GetNumHits();
    nUsedCl += nHits;

    new((*fRiemannTrackArray)[fRiemannTrackArray -> GetEntriesFast()]) STRiemannTrack(*trk);
    for(UInt_t ih = 0; ih < nHits; ++ih){
      STRiemannHit *hit = trk -> GetHit(ih);
      new ((*fRiemannHitArray)[fRiemannHitArray -> GetEntriesFast()]) STRiemannHit(*hit);
    }
  }

  if (fVerbose) {
    std::cerr << "Pattern Reco finished, found tracks: " << foundTrks << "\n";
    std::cerr << "used  " << nUsedCl << " of " << nTotCl << " Clusters \n";
  }

}


void STRiemannTrackingTask::buildTracks(STRiemannTrackFinder *trackfinder,
                                        std::vector<STCluster *> *clusterBuffer,
                                        std::vector<STRiemannTrack *> *TrackletList,
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

  std::vector<STRiemannTrack *> TrackletListCopy = *TrackletList;

  Int_t nTracksIn = TrackletList -> size();
  Int_t nClIn = clusterBuffer -> size();


  STRiemannTrack *LastTrackIn;
  if(nTracksIn > 0)
    LastTrackIn = TrackletList -> back();

  trackfinder -> buildTracks(*clusterBuffer, *TrackletList);

  UInt_t nGoodTrks = 0, nErasedCl = 0, nHits;
  STRiemannTrack *trk;

  for(UInt_t i = 0; i < TrackletList -> size(); ++i){
    trk = (*TrackletList)[i];

    nHits = trk -> GetNumHits();

    if (trk -> distRMS() < maxRMS && (nHits >= minHits || trk -> isGood())) {
      trk -> SetFinished(false);
      trk -> SetGood();
      // clear clusters from good tracklets
      for(UInt_t iCl = 0; iCl < nHits; ++iCl){
        clusterBuffer -> erase(remove(clusterBuffer -> begin(), clusterBuffer -> end(), trk -> GetHit(iCl) -> cluster()),
        clusterBuffer -> end() );
      }
      ++nGoodTrks;

      //push back unique track to riemannlist
      if (std::find(TrackletListCopy.begin(), TrackletListCopy.end(), trk) == TrackletListCopy.end()){
        fRiemannList.push_back(trk);
      }
    }
    else{ // delete bad tracklets
      if (trk -> isGood()){ // track has been found before ( -> clusters were taken out) but does not pass quality criteria anymore  ->  fill clusters back into buffer
        for(UInt_t iCl = 0; iCl < nHits; ++iCl){
          clusterBuffer -> push_back(trk -> GetHit(iCl) -> cluster());
        }
      }
      // delete hits and track
      trk -> deleteHits();
      delete trk;
      TrackletList -> erase(TrackletList -> begin()+i);

      // also has to be removed from fRiemannList
      fRiemannList.erase(remove(fRiemannList.begin(), fRiemannList.end(), trk), fRiemannList.end());
      --i;
    }
  }

  if (fVerbose)
    std::cout << "   found good tracks: " << nGoodTrks - nTracksIn << ", reduced nCl by " << nClIn - clusterBuffer -> size() << std::endl;
}
