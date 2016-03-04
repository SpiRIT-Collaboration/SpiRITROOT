/**
 * @brief Track finder using the riemann circle fit
 *
 * @author Sebastian Neubert (TUM) -- original author
 * @author Johannes Rauch    (TUM)
 * @author Genie Jhang (Korea University)
 * @author JungWoo Lee (Korea University)
 *
 * @detail 
 *    The algorithm can be configured with correlators 
 *    (see STAbsHitTrackCorrelator)
 *    For each hit all track candidates are tested
 *    For each track the algorithm is stepping through the 
 *    correlators which are in a hirarchical order. 
 *    A correlator can be applicable or not. 
 *    If it is applicalble to the hit/track combination and
 *    track survives, then the correlator delivers a matchQuality.
 *    Finally the hit is added to the track candidate that 
 *    reached the deepest correlator level and 
 *    if there are ambiguities achieved the best machtQuality
 */

// SpiRITROOT classes
#include "STHit.hh"
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"
#include "STRiemannTrackFinder.hh"
#include "STRiemannTTCorrelator.hh"
#include "STProximityHTCorrelator.hh"
#include "STRiemannSort.hh"
#include "STGlobal.hh"

// STL
#include <algorithm>
#include <iostream>

// ROOT classes 
#include "TMath.h"

using namespace std;

//#define UPTOHIT
//#define DEBUGHT 1
#ifdef SUBTASK_RIEMANN
#define DEBUGTT 1
#endif
//#define DEBUG 1

// Class Member definitions -----------
STRiemannTrackFinder::STRiemannTrackFinder()
{   
  InitVariables();
}

STRiemannTrackFinder::STRiemannTrackFinder(Double_t scale)
{   
  InitVariables();
  fRiemannScale = scale;
}

STRiemannTrackFinder::~STRiemannTrackFinder()
{
  for(Int_t i = 0; i < fHTCorrelators.size(); i++){
    if(fHTCorrelators[i] != NULL){
      delete fHTCorrelators[i];
      fHTCorrelators[i] = NULL;
    }
  }

  fHTCorrelators.clear();

  for(Int_t i = 0; i < fTTCorrelators.size(); i++){
    if(fTTCorrelators[i] != NULL){
      delete fTTCorrelators[i];
      fTTCorrelators[i] = NULL;
    }
  }

  fTTCorrelators.clear();
}

void
STRiemannTrackFinder::InitVariables()
{
  fMinHitsForFit = 5;
  fSortingMode = kTRUE;
  fSorting = STRiemannSort::kSortR;
  fInteractionZ = 0.;
  fMaxNumHitsForPR = 2147483646;
  fTTProxCut = 500;
  fProxCut = 5;
  fHelixCut = 0.2;
  fRiemannScale = 86.1;
  fMaxR = 0;
  fMinHits = 100;
  fInitTracks = kFALSE;
  fSkipAndDelete = kFALSE;
  fInitDip = 0;
  fInitCurv = 0;
  fSkipCrossingAreas = kFALSE;
}

  const STAbsHitTrackCorrelator *STRiemannTrackFinder::GetHTCorrelator(UInt_t i) const       { return fHTCorrelators.at(i); }
const STAbsTrackTrackCorrelator *STRiemannTrackFinder::GetTTCorrelator(UInt_t i) const       { return fTTCorrelators.at(i); }
                          Int_t  STRiemannTrackFinder::GetSorting()                          { return fSorting; }
                       Double_t  STRiemannTrackFinder::GetInteractionZ()                     { return fInteractionZ; }
                       Double_t  STRiemannTrackFinder::GetScale() const                      { return fRiemannScale; }

void STRiemannTrackFinder::SetMinHitsForFit(UInt_t numHits)                           { fMinHitsForFit = numHits; }
void STRiemannTrackFinder::SetSorting(Int_t sorting)                                  { fSorting = sorting; }
void STRiemannTrackFinder::SetSortingMode(Bool_t sortingMode)                         { fSortingMode = sortingMode; }
void STRiemannTrackFinder::SetInteractionZ(Double_t z)                                { fInteractionZ = z; }
void STRiemannTrackFinder::SetMaxNumHitsForPR(Double_t maxHits)                       { fMaxNumHitsForPR = maxHits; }
void STRiemannTrackFinder::SkipCrossingAreas(Bool_t value)                            { fSkipCrossingAreas = value; }
void STRiemannTrackFinder::SetTTProxCut(Double_t cut)                                 { fTTProxCut = cut; }
void STRiemannTrackFinder::SetProxCut(Double_t cut)                                   { fProxCut = cut; }
void STRiemannTrackFinder::SetHelixCut(Double_t cut)                                  { fHelixCut = cut; }
void STRiemannTrackFinder::InitTracks(Bool_t initTracks, Double_t dip, Double_t curv) { fInitTracks = initTracks; fInitDip = dip; fInitCurv = curv; }
void STRiemannTrackFinder::SetMaxR(Double_t r)                                        { fMaxR = r; }
void STRiemannTrackFinder::SetMinHits(UInt_t minHits)                                 { fMinHits = minHits; }
void STRiemannTrackFinder::SetSkipAndDelete(Bool_t value)                             { fSkipAndDelete = value; }
void STRiemannTrackFinder::SetScale(Double_t scale)                                   { fRiemannScale = scale; }

void
STRiemannTrackFinder::AddHTCorrelator(STAbsHitTrackCorrelator *correlator)
{
  fHTCorrelators.push_back(correlator);
  fFound.push_back(kFALSE);
  fBestMatchQuality.push_back(9999);
  fBestMatchIndex.push_back(0);
}

void
STRiemannTrackFinder::AddTTCorrelator(STAbsTrackTrackCorrelator *correlator)
{
  fTTCorrelators.push_back(correlator);
}


UInt_t
STRiemannTrackFinder::BuildTracks(vector<STHit *> &hitList, vector<STRiemannTrack *> &candList)
{
  #ifdef DEBUGHT
    std::cout << "STRiemannTrackFinder::buildTracks" << std::endl;
  #endif

  UInt_t numFinishedTracks = 0;
  UInt_t numNewTracks = 0;
  UInt_t numHits = hitList.size();
  if (numHits < 3)
    return 0;

  SortHits(hitList);
  const Double_t phi0 = hitList[0] -> GetPosition().Phi();

  Int_t numCorrelators = fHTCorrelators.size();

  #ifdef UPTOHIT
    if (fMaxNumHitsForPR < numHits)
      numHits = fMaxNumHitsForPR;
  #endif

  for (UInt_t iHit = 0; iHit < numHits; iHit++) { // loop over hits
    #ifdef DEBUGHT
        if (iHit%1000 == 0 && iHit > 1) {
          UInt_t hitInTracks = 0;

          for (UInt_t iTracklet = 0; iTracklet < candList.size(); iTracklet++)
            hitInTracks += candList[iTracklet] -> GetNumHits();

          cout << "At hit " << iHit << endl;
          cout << "Tracklets: " << candList.size() << endl;
          cout << "Finished Tracklets: " << numFinishedTracks << endl;
          cout << "Mean number of hits/track: " << (Double_t)hitInTracks/(Double_t)candList.size() << endl;
        }
    #endif


    STRiemannHit *rhit = new STRiemannHit(hitList[iHit], fRiemannScale);
    UInt_t matchTracks = 0;
    UInt_t maxlevel = 0; // index of track with highest number of applicable correlators
    Bool_t foundAtAll = kFALSE; 
    for(Int_t iCor = 0; iCor < numCorrelators; iCor++)
      fBestMatchQuality[iCor] = 99999.; // reset 

    for (UInt_t iTrack = 0; iTrack < candList.size(); iTrack++) { // loop over tracks
#ifdef DEBUG
      cout << "candList.size(): " << candList.size() << " iTrack: " << iTrack << endl;
#endif
      STRiemannTrack *track = candList[iTrack];

      if (track -> IsFinished())
        continue;

      // check if track can be deleted
      Double_t Perp = rhit -> GetHit() -> GetPosition().Perp();
      if (fSkipAndDelete && (iHit%10 == 0 || iHit == numHits - 1)) { // check only every 10 hits, and at last hit
        Bool_t willDelete = kTRUE;
        Bool_t finished = kFALSE;

        if (track -> GetNumHits() > fMinHits || track -> IsGood()) // do not delete
          willDelete = kFALSE;

        // Since the hits are sorted with the sorting criteria,
        // check if the track needed to be compared with the current hit.
        // If the track is far out of the current hit range, make it finish or delete by the condition.
        if (fSorting == STRiemannSort::kSortR) {
          Perp += 3*fProxCut;
          /* Genie 
          std::cout << "Perp: " << Perp << " fProxCut: " << fProxCut << std::endl;
          std::cout << "fPerp: " << track -> GetFirstHit() -> GetHit() -> GetPosition().Perp() << std::endl;
          std::cout << "lPerp: " << track -> GetLastHit() -> GetHit() -> GetPosition().Perp() << std::endl;
          */
          if (track -> GetFirstHit() -> GetHit() -> GetPosition().Perp() > Perp &&
              track -> GetLastHit() -> GetHit() -> GetPosition().Perp() > Perp)
            finished = kTRUE;
        } else if (fSorting == STRiemannSort::kSortPhi) {
          Double_t Phi = rhit -> GetHit() -> GetPosition().Phi();
          Double_t dPhi = 2*fProxCut/Perp; // approx for small angles
          /* Genie
          std::cout << "dPhi: " << dPhi << std::endl;
          std::cout << "fdPhi: " << Phi - track -> GetFirstHit() -> GetHit() -> GetPosition().Phi() << std::endl;
          std::cout << "ldPhi: " << Phi - track -> GetLastHit() -> GetHit() -> GetPosition().Phi() << std::endl;
          */
          if (Phi - track -> GetFirstHit() -> GetHit() -> GetPosition().Phi() > dPhi &&
              Phi - track -> GetLastHit() -> GetHit() -> GetPosition().Phi() > dPhi)
            finished = kTRUE;
        } else if (fSorting == STRiemannSort::kSortReversePhi) {
          Double_t Phi = rhit -> GetHit() -> GetPosition().Phi();
          Double_t dPhi = -2*fProxCut/Perp; // approx for small angles
          /* Genie
          std::cout << "dPhi: " << dPhi << std::endl;
          std::cout << "fdPhi: " << Phi - track -> GetFirstHit() -> GetHit() -> GetPosition().Phi() << std::endl;
          std::cout << "ldPhi: " << Phi - track -> GetLastHit() -> GetHit() -> GetPosition().Phi() << std::endl;
          */
          if (Phi - track -> GetFirstHit() -> GetHit() -> GetPosition().Phi() < dPhi &&
              Phi - track -> GetLastHit() -> GetHit() -> GetPosition().Phi() < dPhi)
            finished = kTRUE;
        } else if (fSorting == STRiemannSort::kSortZ) {
          Double_t Z = rhit -> GetHit() -> GetPosition().Z();
          Double_t dZ = 3*fProxCut;
          /* Genie
          std::cout << "dZ: " << dZ << std::endl;
          std::cout << "fdZ: " << track -> GetFirstHit() -> GetHit() -> GetPosition().Z() - Z << std::endl;
          std::cout << "ldZ: " << track -> GetLastHit() -> GetHit() -> GetPosition().Z() - Z << std::endl;
          */
          if (track -> GetFirstHit() -> GetHit() -> GetPosition().Z() - Z > dZ &&
              track -> GetLastHit() -> GetHit() -> GetPosition().Z() - Z > dZ)
            finished = kTRUE;
        }

        if (finished && willDelete) {
//          std::cout << "Finished and willDelete" << std::endl;
          track -> DeleteHits();
          delete track;
          candList.erase(candList.begin() + iTrack);
          iTrack--;
          continue;
        }

        if (finished) {
//          std::cout << "Finished" << std::endl;
          track -> SetFinished();
          numFinishedTracks++;
          continue;
        }
      }

      // WE STEP THROUGH THE INDIVIDUAL CORRELATORS
      // IF A TRACK SURVIVES EACH CORRELATOR
      // THE HIT IS ASSIGNED TO THE BEST (smallest!) MATCH 
      
      Bool_t trackSurvive = kFALSE;
      vector<Double_t> matchQualities(numCorrelators, 99999.); // for saving the match qualities for each correlator
      Int_t level = 0; // number of survived correlators

      #ifdef DEBUGHT
        if (iHit == fMaxNumHitsForPR - 1)
          std::cout << "Testing hit " << iHit << " with track " << iTrack << "; trk quality: " << track -> GetQuality() << std::endl;
      #endif

      for (Int_t iCor = 0; iCor < numCorrelators; iCor++) { // loop through correlators
        // CORRELATE HIT WITH TRACK
        Double_t matchQuality = 99999;
        Bool_t survive = kFALSE;
        Bool_t applicable = fHTCorrelators[iCor] -> Correlate(track, rhit, survive, matchQuality);
        #ifdef DEBUGHT
          if (iHit == fMaxNumHitsForPR - 1) {
            if (!applicable)
              std::cout << " HT correlator " << iCor << " NOT applicable" << std::endl;
            else
              std::cout << " HT correlator " << iCor << "  IS applicable; survived " << survive << " with MatchQuality " << matchQuality << std::endl;
          }
        #endif

//        std::cout << "applicable: " << ( applicable == kTRUE ? "Applicable" : "Not Applicable" ) << std::endl;
        if (!applicable)
          continue; // try the next correlator

        if (!survive) {
          trackSurvive = kFALSE;
          break; // track has failed this level --> can be excluded
        }
        // track survived this correlator
        level = iCor;
        trackSurvive = kTRUE;
        matchQualities[iCor] = matchQuality;
      } // end loop over correlator


      if (trackSurvive) { // update best values
        // number matching fitted tracks that survived all corrs (for excluding hits)
        if (level == numCorrelators - 1 &&
            !track -> IsInitialized() &&
            track -> GetNumHits() > 3*fMinHitsForFit &&
            matchQualities[numCorrelators - 1] < 0.5*fHelixCut)
          matchTracks++;
        
        if (level > maxlevel)
          maxlevel = level;

        for (UInt_t i = 0; i <= level; i++) {
          if (matchQualities[i] < fBestMatchQuality[i]) {
            fBestMatchQuality[i] = matchQualities[i];
            fBestMatchIndex[i] = iTrack;
          }
        }
      }

      #ifdef DEBUGHT
      if (iHit == fMaxNumHitsForPR - 1 && trackSurvive)
        std::cout << " Track " << iTrack << " survived with level " << level << std::endl;
      if (iHit == fMaxNumHitsForPR - 1)
        std::cout << std::endl;
      #endif

      foundAtAll |= trackSurvive; // foundAtAll will be kTRUE if at least one track survived
    } // end loop over tracks


    #ifdef DEBUGHT
      if (iHit == fMaxNumHitsForPR - 1) {
        std::cout << "maxlevel " << maxlevel << std::endl;
        std::cout << "fBestMatchIndex[maxlevel] " << fBestMatchIndex[maxlevel] << std::endl;
        std::cout << "fBestMatchQuality[maxlevel] " << fBestMatchQuality[maxlevel] << std::endl;
      }
    #endif

    if (!foundAtAll) { // new track if no track survived
      /*if(fSkipAndDelete) {
        Double_t R(rhit -> GetHit() -> GetPosition().Perp());
        if (fSorting == 3 && R<fMaxR){
          #ifdef DEBUGHT
            if(iHit == fMaxNumHitsForPR-1) std::cout << "-> hit perp < " << fMaxR << ", skipping hit " << iHit << std::endl;
          #endif
          continue;
        }
        else if (fSorting == 5 && (rhit -> GetHit() -> GetPosition().Phi()-phi0) * R > 1.){ // todo: hardcoded 2
          #ifdef DEBUGHT
            if(iHit == fMaxNumHitsForPR-1) std::cout << "-> hit (phi-phi0)*R > 1" << ", skipping hit " << iHit << std::endl;
          #endif
          continue;
        }
        else if (fSorting == -5 && (rhit -> GetHit() -> GetPosition().Phi()-phi0) * R < -1.){ // todo: hardcoded 2
          #ifdef DEBUGHT
            if(iHit == fMaxNumHitsForPR-1) std::cout << "-> hit (phi-phi0)*R < -1" << ", skipping hit " << iHit << std::endl;
          #endif
          continue;
        }
      }*/

      numNewTracks++;
      STRiemannTrack *track = new STRiemannTrack(fRiemannScale);
      track -> SetSort(fSortingMode);
      candList.push_back(track);
      //std::cout << "Creating new track" << std::endl;
      track -> AddHit(rhit);
      #ifdef DEBUGHT
        if (iHit == fMaxNumHitsForPR - 1)
          std::cout << "-> creating new track Nr " << candList.size() - 1 << std::endl;
      #endif

      if (fInitTracks) {
        if (fSorting == STRiemannSort::kSortR)
          track -> InitTargetTrack(fInitDip, fInitCurv);

        if (fSorting == STRiemannSort::kSortPhi || fSorting == STRiemannSort::kSortReversePhi)
          track -> InitCircle(0);
      }
    } else {
      // if more than one track matches, hit lies in crossing section -> skip
      if (fSkipCrossingAreas && matchTracks > 1) {
        #ifdef DEBUGHT
          if (iHit == fMaxNumHitsForPR - 1)
            std::cout << " " << matchTracks << " tracks match -> hit lies in crossing area -> skip hit" << std::endl;
        #endif        
        ResetFlags();

        delete rhit;

        hitList.erase(hitList.begin() + iHit);
        iHit--;
        numHits--;
        continue;
      }
      // add hit to best match
      // use the bestMatch from deepest level
      STRiemannTrack *theTrack = candList[fBestMatchIndex[maxlevel]];
      #ifdef DEBUGHT
        if (iHit == fMaxNumHitsForPR - 1)
          std::cout << "-> adding hit to track" << fBestMatchIndex[maxlevel] << std::endl;
      #endif
      theTrack -> AddHit(rhit);
      if (theTrack -> GetNumHits() >= fMinHitsForFit) {
        theTrack -> FitAndSort();
        #ifdef DEBUGHT
          if (iHit == fMaxNumHitsForPR - 1) {
           std::cout << " track parameters: fC=" << theTrack -> GetC() << "  R=" << theTrack -> GetR() << "  dip °=" << (theTrack -> GetDip())*180/TMath::Pi() << std::endl;
           std::cout << " center: ";
           theTrack -> GetCenter().Print();
          }
        #endif
      }
    }
    ResetFlags();
  } // end loop over hits
  

  #ifdef DEBUGHT
    std::cout << candList.size() << " Riemann Tracks found." << std::endl;
  #endif

 return numNewTracks;
}

void
STRiemannTrackFinder::MergeTracks(vector<STRiemannTrack *> &candList){
  UInt_t numTracks = candList.size();
  #ifdef DEBUGTT
  std::cout << "STRiemannTrackFinder::mergeTracks: " << numTracks << " track to merge\n";
  #endif

  if (numTracks < 2)
    return; // need at least 2 trackcands to merge

  // sort tracklets, but use different sorting than for hits!
  SortTracklets(candList);

  Double_t z1max, z2min, zTemp;

  for (UInt_t iTrack1 = 0; iTrack1 < numTracks - 1; iTrack1++) { // loop over tracks
    if (candList[iTrack1] == NULL)
      continue;

    STRiemannTrack *track1 = candList[iTrack1];

    // find max z of track1
    if (fSorting == STRiemannSort::kSortR) {
      z1max = track1 -> GetFirstHit() -> GetHit() -> GetPosition().Z();
      zTemp = track1 -> GetLastHit() -> GetHit() -> GetPosition().Z();

      if (zTemp > z1max) 
        z1max = zTemp;
    }

    for (UInt_t iTrack2 = iTrack1 + 1; iTrack2 < numTracks; iTrack2++) { // loop over the other tracks to be tested
      if(candList[iTrack2] == NULL)
        continue;

      #ifdef DEBUGTT
        std::cout << "Testing track " << iTrack1 << " with track " << iTrack2 << std::endl;
      #endif

      STRiemannTrack *track2 = candList[iTrack2];

      // find min z of track2
      if(fSorting == STRiemannSort::kSortR){
        z2min = track2 -> GetFirstHit() -> GetHit() -> GetPosition().Z();
        zTemp = track2 -> GetLastHit() -> GetHit() -> GetPosition().Z();

        if (zTemp < z2min)
          z2min = zTemp;

        // tracklets are sorted by z (from small to big), if the smallest z of the track2 is bigger than the maximum z of track1, skip all other tracks
        if (z2min > (z1max + fTTProxCut + 1.)) {
          #ifdef DEBUGTT
            std::cout << " (z2min > (z1max + fTTProxCut + 0.1) ), skipping rest of track2 tracklets (" << numTracks - iTrack2 << ")" << std::endl;
          #endif
          break; // continue with next track1
        }
      }

      // WE STEP THROUGH THE INDIVIDUAL CORRELATORS
      // IF THE TRACK track2 SURVIVES EACH CORRELATOR
      // IT IS MERGED WITH THE TRACK track1
      Bool_t survive = kTRUE;
      for (UInt_t iCor = 0; iCor < fTTCorrelators.size(); iCor++) { // loop through correlators
        // CORRELATE track1 WITH track2
        // all correlators must be applicable and must be survived!
        Double_t matchQuality;
        Bool_t applicable;

        // make sure that the first track in the correlator is the bigger one
        if (track1 -> GetNumHits() >= track2 -> GetNumHits())
          applicable = fTTCorrelators[iCor] -> Correlate(track1, track2, survive, matchQuality);
        else
          applicable = fTTCorrelators[iCor] -> Correlate(track2, track1, survive, matchQuality);

        #ifdef DEBUGTT
          if (!applicable)
            std::cout << " TT correlator " << iCor << " NOT applicable" << std::endl;
          else {
            std::cout << " TT correlator " << iCor << "  IS applicable" << std::endl;

            if (!survive)
              std::cout << " -> Dead! with MatchQuality " << matchQuality << std::endl;
            else
              std::cout << " -> Survive! with MatchQuality " << matchQuality << std::endl;
          }
        #endif

        if (!applicable) {
          survive = kFALSE;
          break;
        }

        if (!survive)
          break;
      } // end loop through correlators

      if (!survive)
        continue; // test next track2

      // merge tracks if survived
      #ifdef DEBUGTT
        std::cout << "merge track " << iTrack1 << " with track " << iTrack2 << std::endl;
      #endif

      UInt_t nhits1 = track1 -> GetNumHits();
      UInt_t nhits2 = track2 -> GetNumHits();

      if (!fSortingMode) { // we have to collect hits from both tracks, sort them and build a new track
        // collect hits from both tracks and sort
        vector<STHit *> hits;
        for(UInt_t iHit = 0; iHit < nhits1; iHit++)
          hits.push_back(track1 -> GetHit(iHit) -> GetHit());

        for(UInt_t iHit = 0; iHit < nhits2; iHit++)
          hits.push_back(track2 -> GetHit(iHit) -> GetHit());
        
        SortHits(hits);

        // fill hits into new RiemannTrack and refit
        STRiemannTrack *mergedTrack = new STRiemannTrack(fRiemannScale);
        mergedTrack -> SetSort(kFALSE);
        for (UInt_t iHit = 0; iHit < hits.size(); iHit++) {
          STRiemannHit *rhit = new STRiemannHit(hits[iHit], fRiemannScale);
          mergedTrack -> AddHit(rhit);
        }

        if (mergedTrack -> GetNumHits() >= fMinHitsForFit)
          mergedTrack -> FitAndSort();

        // delete old trackcands and store new trackcand
        delete candList[iTrack1];

        track1 = mergedTrack;
        candList[iTrack1] = mergedTrack;
      } else { // we can just add the hits from track2 to track1 and the sorting is done internally
        for (UInt_t iHit = 0; iHit < nhits2; iHit++)
          track1 -> AddHit(track2 -> GetHit(iHit));

        track1 -> SetSort(kTRUE);

        // refit if we have enough hits
        if (track1 -> GetNumHits() >= fMinHitsForFit)
          track1 -> FitAndSort();
      }

      // update max z of track1
      if (fSorting == STRiemannSort::kSortR) {
        z1max = track1 -> GetFirstHit() -> GetHit() -> GetPosition().Z();
        zTemp = track1 -> GetLastHit() -> GetHit() -> GetPosition().Z();
        if (zTemp>z1max) z1max = zTemp;
      }

      // delete track2
      delete candList[iTrack2];
      candList[iTrack2] = NULL;

    } // end loop over the other tracks to be tested
  } // end loop over tracks

  // clean up candList
  for (Int_t iCand = 0; iCand < candList.size(); iCand++) {
    if (candList[iCand] == NULL) {
      candList.erase(candList.begin() + iCand);
      iCand--; // go one step back because "erase" shifts back the rest
    }
  }

  #ifdef DEBUGTT
    std::cout << candList.size() << " Merged Riemann Tracks" << std::endl;
  #endif
}


void
STRiemannTrackFinder::CleanTracks(vector<STRiemannTrack *> &candList, Double_t szcut, Double_t planecut)
{
  std::cout << "WARNING: STRiemannTrackFinder::cleanTracks - no functionality!" << std::endl;
  /*STRiemannHit *hit;

  for(UInt_t i = 0; i<candList.size(); ++i){ // loop over trackcands
    if(!candList[i] -> IsFitted() || !candList[i] -> IsFitted()) continue; // skip track

    for(UInt_t j = 0; j<candList[i] -> GetNumHits(); ++j){ // loop over hits
      hit = candList[i] -> GetHit(j);
      if (TMath::Abs(candList[i] -> dist(hit)) > planecut || TMath::Abs(candList[i] -> szDist(hit)) > szcut){
        candList[i] -> removeHit(j);
        candList[i] -> refit();
        --j;
      }
      if(!candList[i] -> IsFitted() || !candList[i] -> IsFitted()) break; // skip track
    } // end loop over hits

  } // end loop over trackcands*/
}


void
STRiemannTrackFinder::SortHits(vector<STHit *> &hitList)
{
  if(fSorting == STRiemannSort::kNoSort)
    return;

  SortHitClass sortHit;
  sortHit.SetSorting(fSorting);
  sortHit.SetInteractionZ(fInteractionZ);
  std::sort(hitList.begin(), hitList.end(), sortHit);
}

void
STRiemannTrackFinder::SortTracklets(vector<STRiemannTrack *> &tracklets){
  if(fSorting == STRiemannSort::kNoSort)
    return;

  SortTrackletsClass sortTracklet;
  sortTracklet.SetSorting(fSorting);
  std::sort(tracklets.begin(), tracklets.end(), sortTracklet);
}


void
STRiemannTrackFinder::ResetFlags()
{
  // reset all flags
  for(Int_t k = 0; k < fHTCorrelators.size(); k++) {
    fFound[k] = kFALSE;
    fBestMatchQuality[k] = 99999;
    fBestMatchIndex[k] = 0;
  }
}


Bool_t
SortHitClass::operator() (STHit *s1, STHit *s2)
{
  Double_t a1;
  Double_t a2;
  TVector3 d1;
  TVector3 d2;

  switch (fSorting) {
    case -1: //no sorting
      return kFALSE;
      break;

    case 0:
      a1 = s1 -> GetPosition().X();
      a2 = s2 -> GetPosition().X();
      return a1 > a2;
      break;

    case 1:
      a1 = s1 -> GetPosition().Y();
      a2 = s2 -> GetPosition().Y();
      return a1 > a2;
      break;

    case 2:
      a1 = s1 -> GetPosition().Z();
      a2 = s2 -> GetPosition().Z();
      return a1 > a2;
      break;

    case 4:
      d1 = s1 -> GetPosition();
      d1(2) -= fInteractionZ;
      d2 = s2 -> GetPosition();
      d2(2) -= fInteractionZ;
      a1 = d1.Mag();
      a2 = d2.Mag();
      return a1 > a2;
      break;

    case 5:
      a1 = s1 -> GetPosition().Phi();
      a2 = s2 -> GetPosition().Phi();
      if (a1 < -1.*TMath::PiOver2()) a1 += TMath::TwoPi();
      if (a2 < -1.*TMath::PiOver2()) a2 += TMath::TwoPi();
      return a1 < a2;
      break;
       
    case -5:
      a1 = s1 -> GetPosition().Phi();
      a2 = s2 -> GetPosition().Phi();
      if (a1 < -1.*TMath::PiOver2()) a1 += TMath::TwoPi();
      if (a2 < -1.*TMath::PiOver2()) a2 += TMath::TwoPi();
      return a1 > a2;
      break;

    case 3:
    default:
      a1 = s1 -> GetPosition().Perp();
      a2 = s2 -> GetPosition().Perp();
      return a1 > a2;
  }
}


Bool_t
SortTrackletsClass::operator() (STRiemannTrack *t1, STRiemannTrack *t2)
{
  Double_t a1;
  Double_t a12;
  Double_t a2;
  Double_t a22;
  TVector3 d1;
  TVector3 d2;

  switch (fSorting) {
    case STRiemannSort::kNoSort: //no sorting
      return kFALSE;

    // if hits are NOT sorted by R, sort tracklets by R
    case STRiemannSort::kSortX:
    case STRiemannSort::kSortY:
    case STRiemannSort::kSortZ:
    case STRiemannSort::kSortDistance:
      a1 = t1 -> GetFirstHit() -> GetHit() -> GetPosition().Perp();
      a12 = t1 -> GetLastHit() -> GetHit() -> GetPosition().Perp();
      if (a12 < a1) a1 = a12;

      a2 = t2 -> GetFirstHit() -> GetHit() -> GetPosition().Perp();
      a22 = t2 -> GetLastHit() -> GetHit() -> GetPosition().Perp();
      if (a22 < a2) a2 = a22;

      return a1 < a2;

    // if hits are sorted by R, sort tracklets by Z
    case STRiemannSort::kSortR:
    default:
      a1 = t1 -> GetFirstHit() -> GetHit() -> GetPosition().Z();
      a12 = t1 -> GetLastHit() -> GetHit() -> GetPosition().Z();
      if (a12 < a1) a1 = a12;

      a2 = t2 -> GetFirstHit() -> GetHit() -> GetPosition().Z();
      a22 = t2 -> GetLastHit() -> GetHit() -> GetPosition().Z();
      if (a22 < a2) a2 = a22;

      return a1 < a2;
  }
}
