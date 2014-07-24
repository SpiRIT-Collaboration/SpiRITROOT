//-----------------------------------------------------------
// Description:
//      Track finder using the riemann circle fit
//
//      The algorithm can be configured with correlators 
//      (see STAbsHitTrackCorrelator)
//      For each hit all track candidates are tested
//      For each track the algorithm is stepping through the 
//      correlators which are in a hirarchical order. 
//      A correlator can be applicable or not. 
//      If it is applicalble to the hit/track combination and
//      the track survives, then the correlator delivers a matchQuality.
//      Finally the hit is added to the track candidate that 
//      reached the deepest correlator level and 
//      if there are ambiguities achieved the best machtQuality
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Original Author List:
//      Sebastian Neubert    TUM            (original author)
//      Johannes Rauch       TUM
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------
//
// SpiRITROOT classes
#include "STRiemannTrack.h"
#include "STRiemannHit.h"
#include "STRiemannTrackFinder.hh"
#include "STRiemannTTCorrelator.h"
#include "STProximityHTCorrelator.h"

// STL
#include <algorithm>
#include <iostream>

// 
#include "DebugLogger.h"
#include "GFTrackCand.h"
#include "TpcCluster.h"
#include "TMath.h"

using namespace std;

#define UPTOHIT

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
  for(Int_t i=0;i<fHTCorrelators.size();++i){
    if(fHTCorrelators[i]!=NULL){
      delete fHTCorrelators[i];
      fHTCorrelators[i]=NULL;
    }
  }
  fHTCorrelators.clear();

  for(Int_t i=0;i<fTTCorrelators.size();++i){
    if(fTTCorrelators[i]!=NULL){
      delete fTTCorrelators[i];
      fTTCorrelators[i]=NULL;
    }
  }
  fTTCorrelators.clear();
}

void
STRiemannTrackFinder::InitVariables()
{
  fMinHitsForFit = 5;
  fSortingMode = kTRUE;
  fSorting = 3;
  fInteractionZ = 0.;
  fMaxNumHitsForPR = 2147483646;
  fTTproxcut = 500;
  fProxcut = 5;
  fHelixcut = 0.2;
  fRiemannScale = 24.6;
  fMaxR = 0;
  fMinHits = 100;
  fInitTrks = kFALSE;
  fSkipAndDelete = kFALSE;
  fInitDip = 0;
  fInitCurv = 0;
  fSkipCrossingAreas = kFALSE;
}

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
STRiemannTrackFinder::buildTracks(vector<TpcCluster*>& cll,
				   vector<STRiemannTrack*>& candlist)
{
  #ifdef DEBUGHT
    std::cout<<"STRiemannTrackFinder::buildTracks"<<std::endl;
  #endif

  UInt_t nFinishedTrks(0);
  UInt_t nNewTracks(0);
  UInt_t ncl(cll.size());
  if(ncl<3) return 0;

  sortClusters(cll);
  const Double_t phi0(cll[0]->pos().Phi());

  Int_t ncor = fHTCorrelators.size();

  #ifdef UPTOHIT
    if(fMaxNumHitsForPR<ncl) ncl=fMaxNumHitsForPR;
  #endif

  for(UInt_t icl=0;icl<ncl;++icl){ // loop over hits
    #ifdef DEBUGHT
        if(icl%1000==0 && icl > 1){
          UInt_t clInTrks(0);
          for (UInt_t itrklt=0; itrklt<candlist.size(); ++itrklt){
            clInTrks += candlist[itrklt]->getNumHits();
          }
          cout << "At cluster " << icl << endl;
          cout << "Tracklets: "<< candlist.size() << endl;
          cout << "Finished Tracklets: "<< nFinishedTrks << endl;
          cout << "Mean number of hits/track: "<< (Double_t)clInTrks/(Double_t)candlist.size() << endl;
        }
    #endif


    STRiemannHit* rhit=new STRiemannHit(cll[icl],fRiemannScale);
    UInt_t matchTrks(0);
    UInt_t maxlevel=0; // index of track with highest number of applicable correlators
    Bool_t foundAtAll=kFALSE; 
    for(Int_t i=0;i<ncor;++i) fBestMatchQuality[i] = 99999.;// reset 

    for(UInt_t itrk=0;itrk<candlist.size();++itrk){ // loop over tracks
      STRiemannTrack* trk=candlist[itrk];
      if (trk->isFinished()) continue;

      // check if track can be deleted
      Double_t Perp(rhit->cluster()->pos().Perp());
      if(fSkipAndDelete && (icl%10 == 0 || icl == ncl-1)) { // check only every 10 hits, and at last hit
        Bool_t deleet(kTRUE), finished(kFALSE);

        if (trk->getNumHits() > fMinHits || trk->isGood()){ // do not delete
          deleet=kFALSE;
        }

        if (fSorting==3){
          Perp += 3*fProxcut;
          if (trk->getFirstHit()->cluster()->pos().Perp() > Perp &&
              trk->getLastHit()->cluster()->pos().Perp() > Perp)
            finished = kTRUE;
        }
        else if (fSorting==5){
          Double_t Phi(rhit->cluster()->pos().Phi());
          Double_t dPhi = 2*fProxcut/Perp; // approx for small angles
          if (Phi - trk->getFirstHit()->cluster()->pos().Phi() > dPhi &&
              Phi - trk->getLastHit()->cluster()->pos().Phi() > dPhi)
            finished = kTRUE;
        }
        else if (fSorting==-5){
          Double_t Phi(rhit->cluster()->pos().Phi());
          Double_t dPhi = -2*fProxcut/Perp; // approx for small angles
          if (Phi - trk->getFirstHit()->cluster()->pos().Phi() < dPhi &&
              Phi - trk->getLastHit()->cluster()->pos().Phi() < dPhi)
            finished = kTRUE;
        }
        else if (fSorting==2){
          Double_t Z(rhit->cluster()->pos().Z());
          Double_t dZ = 3*fProxcut;
          if (trk->getFirstHit()->cluster()->pos().Z() - Z > dZ &&
              trk->getLastHit()->cluster()->pos().Z() - Z > dZ)
            finished = kTRUE;
        }


        if(finished && deleet){
          trk->deleteHits();
          delete trk;
          candlist.erase(candlist.begin()+itrk);
          --itrk;
          continue;
        }

        if(finished) {
          trk->SetFinished();
          ++nFinishedTrks;
          continue;
        }
      }


      // WE STEP THROUGH THE INDIVIDUAL CORRELATORS
      // IF A TRACK SURVIVES EACH CORRELATOR
      // THE HIT IS ASSIGNED TO THE BEST (smallest!) MATCH 
      
      Bool_t trksurvive = kFALSE;
      vector<Double_t> matchQualities(ncor, 99999.); // for saving the match qualities for each correlator
      Int_t level = 0; // number of survived correlators
      #ifdef DEBUGHT
        if(icl==fMaxNumHitsForPR-1) std::cout<<"Testing hit "<<icl<<" with track "<<itrk<< "; trk quality: " << trk->quality() << std::endl;
      #endif

      for(Int_t icor=0; icor<ncor; ++icor){ // loop through correlators
        // CORRELATE HIT WITH TRACK
        Double_t matchQuality = 99999;
        Bool_t survive=kFALSE;
        Bool_t applicable=fHTCorrelators[icor]->corr(trk,rhit,survive,matchQuality);
        #ifdef DEBUGHT
          if(icl==fMaxNumHitsForPR-1){
            if(!applicable){std::cout<<"  correlator "<<icor<<" NOT applicable"<<std::endl;}
            else{std::cout<<"  correlator "<<icor<<"  IS applicable; survived "<<survive<<" with MatchQuality "<<matchQuality<<std::endl;}
          }
        #endif
        if(!applicable) continue; // try the next correlator
        if(!survive){
          trksurvive=kFALSE;
          break; // track has failed this level --> can be excluded
        }
        // track survived this correlator
        level = icor;
        trksurvive = kTRUE;
        matchQualities[icor] = matchQuality;
      } // end loop over correlator


      if(trksurvive){ // update best values
        // number matching fitted tracks that survived all corrs (for excluding clusters)
        if(level==ncor-1 && !trk->isInitialized() &&
           trk->getNumHits() > 3*fMinHitsForFit &&
           matchQualities[ncor-1] < 0.5*fHelixcut) ++matchTrks;
        
        if(level>maxlevel) maxlevel=level;
        for(UInt_t i=0; i<=level; ++i){
          if(matchQualities[i]<fBestMatchQuality[i]){
            fBestMatchQuality[i]=matchQualities[i];
            fBestMatchIndex[i]=itrk;
          }
        }
      }

      #ifdef DEBUGHT
      if(icl==fMaxNumHitsForPR-1 && trksurvive) std::cout<<" Track "<<itrk<<" survived with level "<<level<<std::endl;
        if(icl==fMaxNumHitsForPR-1) std::cout<<std::endl;
      #endif

      foundAtAll|=trksurvive; // foundAtAll will be kTRUE if at least one track survived
    } // end loop over tracks


    #ifdef DEBUGHT
      if(icl==fMaxNumHitsForPR-1){
        std::cout<<"maxlevel "<< maxlevel <<std::endl;
        std::cout<<"fBestMatchIndex[maxlevel] "<<fBestMatchIndex[maxlevel] <<std::endl;
        std::cout<<"fBestMatchQuality[maxlevel] "<<fBestMatchQuality[maxlevel] <<std::endl;
      }
    #endif


    if(!foundAtAll){ // new track if no track survived
      /*if(fSkipAndDelete) {
        Double_t R(rhit->cluster()->pos().Perp());
        if (fSorting==3 && R<fMaxR){
          #ifdef DEBUGHT
            if(icl==fMaxNumHitsForPR-1) std::cout<<"-> hit perp < " << fMaxR << ", skipping hit "<<icl<<std::endl;
          #endif
          continue;
        }
        else if (fSorting==5 && (rhit->cluster()->pos().Phi()-phi0) * R > 1.){ // todo: hardcoded 2
          #ifdef DEBUGHT
            if(icl==fMaxNumHitsForPR-1) std::cout<<"-> hit (phi-phi0)*R > 1" << ", skipping hit "<<icl<<std::endl;
          #endif
          continue;
        }
        else if (fSorting==-5 && (rhit->cluster()->pos().Phi()-phi0) * R < -1.){ // todo: hardcoded 2
          #ifdef DEBUGHT
            if(icl==fMaxNumHitsForPR-1) std::cout<<"-> hit (phi-phi0)*R < -1" << ", skipping hit "<<icl<<std::endl;
          #endif
          continue;
        }
      }*/

      ++nNewTracks;
      STRiemannTrack* trk=new STRiemannTrack(fRiemannScale);
      trk->SetSort(fSortingMode);
      candlist.push_back(trk);
      //std::cout<<"Creating new track"<<std::endl;
      trk->addHit(rhit);
      #ifdef DEBUGHT
        if(icl==fMaxNumHitsForPR-1) std::cout<<"-> creating new track Nr "<<candlist.size()-1<<std::endl;
      #endif

      if(fInitTrks) {
        if (fSorting==3) trk->initTargetTrack(fInitDip, fInitCurv);
        if (fSorting==5 || fSorting==-5) trk->initCircle(0);
      }

    }
    else {
      // if more than one track matches, hit lies in crossing section -> skip
      if(fSkipCrossingAreas && matchTrks>1) {
        #ifdef DEBUGHT
          if(icl==fMaxNumHitsForPR-1){
           std::cout<<" "<<matchTrks<<" tracks match -> hit lies in crossing area -> skip hit"<<std::endl;
          }
        #endif        
        resetFlags();
        delete rhit;
        cll.erase(cll.begin() + icl);
        --icl;
        --ncl;
        continue;
      }
      // add hit to best match
      // use the bestMatch from deepest level
      STRiemannTrack* theTrk=candlist[fBestMatchIndex[maxlevel]];
      #ifdef DEBUGHT
        if(icl==fMaxNumHitsForPR-1) std::cout<<"-> adding hit to track"<<fBestMatchIndex[maxlevel]<<std::endl;
      #endif
      theTrk->addHit(rhit);
      if(theTrk->getNumHits()>=fMinHitsForFit){
        theTrk->fitAndSort();
        #ifdef DEBUGHT
          if(icl==fMaxNumHitsForPR-1){
           std::cout<<" track parameters: fC="<<theTrk->c()<<"  R="<<theTrk->r()<<"  dip °="<<(theTrk->dip())*180/TMath::Pi()<<std::endl;
           std::cout<<" center: "; theTrk->center().Print();
          }
        #endif
      }
    }
    resetFlags();
  } // end loop over hits
  

  #ifdef DEBUGHT
    std::cout<<candlist.size()<<" Riemann Tracks found."<<std::endl;
  #endif

 return nNewTracks;
}

void
STRiemannTrackFinder::mergeTracks(vector<STRiemannTrack*>& candlist){
  #ifdef DEBUGTT
    std::cout<<"STRiemannTrackFinder::mergeTracks"<<std::endl;
  #endif

  UInt_t ntr=candlist.size();
  #ifdef DEBUGTT
  std::cout<<"STRiemannTrackFinder::mergeTracks: "<<ntr<<" track to merge\n";
  #endif
  if (ntr<2) return; // need at least 2 trackcands to merge

  // sort tracklets, but use different sorting than for clusters!
  sortTracklets(candlist);

  Double_t z1max, z2min, zTemp;

  for(UInt_t itrk1=0; itrk1<ntr-1; ++itrk1){ // loop over tracks
    if(candlist[itrk1]==NULL)continue;
    STRiemannTrack* trk1=candlist[itrk1];

    // find max z of trk1
    if(fSorting==3){
      z1max = trk1->getFirstHit()->cluster()->pos().Z();
      zTemp = trk1->getLastHit()->cluster()->pos().Z();
      if (zTemp>z1max) z1max=zTemp;
    }

    for(UInt_t itrk2=itrk1+1; itrk2<ntr; ++itrk2){ // loop over the other tracks to be tested
      if(candlist[itrk2]==NULL)continue;

      #ifdef DEBUGTT
        std::cout<<"Testing track "<<itrk1<<" with track "<<itrk2<<std::endl;
      #endif

      STRiemannTrack* trk2=candlist[itrk2];

      // find min z of trk2
      if(fSorting==3){
        z2min = trk2->getFirstHit()->cluster()->pos().Z();
        zTemp = trk2->getLastHit()->cluster()->pos().Z();
        if (zTemp<z2min) z2min=zTemp;
        // tracklets are sorted by z (from small to big), if the smallest z of the trk2 is bigger than the maximum z of trk1, skip all other tracks
        if (z2min > (z1max + fTTproxcut + 0.1) ) {
          #ifdef DEBUGTT
            std::cout<<" (z2min > (z1max + fTTproxcut + 0.1) ), skipping rest of trk2 tracklets (" << ntr-itrk2 << ")" <<std::endl;
          #endif
          break; // continue with next trk1
        }
      }

      // WE STEP THROUGH THE INDIVIDUAL CORRELATORS
      // IF THE TRACK trk2 SURVIVES EACH CORRELATOR
      // IT IS MERGED WITH THE TRACK trk1
      Bool_t survive=kTRUE;
      for(UInt_t icor=0; icor<fTTCorrelators.size(); ++icor){ // loop through correlators
        // CORRELATE trk1 WITH trk2
        // all correlators must be applicable and must be survived!
        Double_t matchQuality;
        Bool_t applicable;

        // make sure that the first track in the correlator is the bigger one
        if (trk1->getNumHits()>=trk2->getNumHits())
          applicable=fTTCorrelators[icor]->corr(trk1,trk2,survive,matchQuality);
        else
          applicable=fTTCorrelators[icor]->corr(trk2,trk1,survive,matchQuality);

        #ifdef DEBUGTT
          if(!applicable){std::cout<<"  correlator "<<icor<<" NOT applicable"<<std::endl;}
          else{std::cout<<"  correlator "<<icor<<"  IS applicable; survived "<<survive<<" with MatchQuality "<<matchQuality<<std::endl;}
        #endif

        if(!applicable) {
          survive = kFALSE;
          break;
        }
        if(!survive) break;
      } // end loop through correlators
      if (!survive) continue; // test next trk2

      // merge tracks if survived
      #ifdef DEBUGTT
        std::cout<<"merge track "<<itrk1<<" with track "<<itrk2<<std::endl;
      #endif

      UInt_t nhits1 = trk1->getNumHits();
      UInt_t nhits2 = trk2->getNumHits();

      if(!fSortingMode){ // we have to collect clusters from both tracks, sort them and build a new track
        // collect clusters from both tracks and sort
        vector<TpcCluster*> clusters;
        for(UInt_t i=0; i<nhits1; ++i){
          clusters.push_back(trk1->getHit(i)->cluster());
        }
        for(UInt_t i=0; i<nhits2; ++i){
          clusters.push_back(trk2->getHit(i)->cluster());
        }
        sortClusters(clusters);

        // fill clusters into new RiemannTrack and refit
        STRiemannTrack* mergedTrack = new STRiemannTrack(fRiemannScale);
        mergedTrack->SetSort(kFALSE);
        for(UInt_t i=0; i<clusters.size(); ++i){
          STRiemannHit* rhit = new STRiemannHit(clusters[i],
							fRiemannScale);
          mergedTrack->addHit(rhit);
        }
        if(mergedTrack->getNumHits()>=fMinHitsForFit) mergedTrack->fitAndSort();

        // delete old trackcands and store new trackcand
        delete candlist[itrk1];
        trk1=mergedTrack;
        candlist[itrk1]=mergedTrack;
      }
      else { // we can just add the hits from trk2 to trk1 and the sorting is done internally
        for(UInt_t i=0; i<nhits2; ++i)
          trk1->addHit(trk2->getHit(i));
        trk1->SetSort(kTRUE);

        // refit if we have enough hits
        if(trk1->getNumHits()>=fMinHitsForFit) trk1->fitAndSort();
      }

      // update max z of trk1
      if(fSorting==3){
        z1max = trk1->getFirstHit()->cluster()->pos().Z();
        zTemp = trk1->getLastHit()->cluster()->pos().Z();
        if (zTemp>z1max) z1max=zTemp;
      }

      // delete trk2
      delete candlist[itrk2];
      candlist[itrk2]=NULL;

    } // end loop over the other tracks to be tested
  } // end loop over tracks

  // clean up candlist
  for(Int_t i=0; i<candlist.size(); ++i){
    if (candlist[i]==NULL) {
      candlist.erase(candlist.begin()+i);
      --i; // go one step back because "erase" shifts back the rest
    }
  }

  #ifdef DEBUGTT
    std::cout<<candlist.size()<<" Merged Riemann Tracks"<<std::endl;
  #endif
}


void
STRiemannTrackFinder::cleanTracks(vector<STRiemannTrack*>& candlist,
                                      Double_t szcut, Double_t planecut){

  std::cout<<"WARNING: STRiemannTrackFinder::cleanTracks - no functionality!"<<std::endl;
  /*STRiemannHit* hit;

  for(UInt_t i=0; i<candlist.size(); ++i){ // loop over trackcands
    if(!candlist[i]->isFitted() || !candlist[i]->isFitted()) continue; // skip track

    for(UInt_t j=0; j<candlist[i]->getNumHits(); ++j){ // loop over hits
      hit = candlist[i]->getHit(j);
      if (TMath::Abs(candlist[i]->dist(hit)) > planecut || TMath::Abs(candlist[i]->szDist(hit)) > szcut){
        candlist[i]->removeHit(j);
        candlist[i]->refit();
        --j;
      }
      if(!candlist[i]->isFitted() || !candlist[i]->isFitted()) break; // skip track
    } // end loop over hits

  } // end loop over trackcands*/

}


void
STRiemannTrackFinder::sortClusters(vector<TpcCluster*>& cll){
  if(fSorting==-1) return;
  sortClusterClass sortCluster;
  sortCluster.SetSorting(fSorting);
  sortCluster.SetInteractionZ(fInteractionZ);
  std::sort(cll.begin(),cll.end(),sortCluster);
}

void
STRiemannTrackFinder::sortTracklets(vector<STRiemannTrack*>& tracklets){
  if(fSorting==-1) return;
  sortTrackletsClass sortTracklet;
  sortTracklet.SetSorting(fSorting);
  std::sort(tracklets.begin(),tracklets.end(),sortTracklet);
}


void
STRiemannTrackFinder::resetFlags(){
// reset all flags
  for(Int_t k=0;k<fHTCorrelators.size();++k){
    fFound[k]=kFALSE;
    fBestMatchQuality[k]=99999;
    fBestMatchIndex[k]=0;
  }
}


Bool_t
sortClusterClass::operator() (TpcCluster* s1, TpcCluster* s2){
  Double_t a1;
  Double_t a2;
  TVector3 d1;
  TVector3 d2;
  switch (sorting){
    case -1: //no sorting
      return kFALSE;
      break;
    case 0:
      a1=s1->pos().X();
      a2=s2->pos().X();
      return a1>a2;
      break;
    case 1:
      a1=s1->pos().Y();
      a2=s2->pos().Y();
      return a1>a2;
      break;
    case 2:
      a1=s1->pos().Z();
      a2=s2->pos().Z();
      return a1>a2;
      break;
    case 4:
      d1 = s1->pos();
      d1(2) -= interactionZ;
      d2 = s2->pos();
      d2(2) -= interactionZ;
      a1=d1.Mag();
      a2=d2.Mag();
      return a1>a2;
      break;
    case 5:
      a1=s1->pos().Phi();
      a2=s2->pos().Phi();
      if (a1 < -1.*TMath::PiOver2()) a1 += TMath::TwoPi();
      if (a2 < -1.*TMath::PiOver2()) a2 += TMath::TwoPi();
      return a1<a2;
      break;
    case -5:
      a1=s1->pos().Phi();
      a2=s2->pos().Phi();
      if (a1 < -1.*TMath::PiOver2()) a1 += TMath::TwoPi();
      if (a2 < -1.*TMath::PiOver2()) a2 += TMath::TwoPi();
      return a1>a2;
      break;
    case 3:
    default:
      a1=s1->pos().Perp();
      a2=s2->pos().Perp();
      return a1>a2;
  }
}


Bool_t
sortTrackletsClass::operator() (STRiemannTrack* t1, STRiemannTrack* t2){
  Double_t a1;
  Double_t a12;
  Double_t a2;
  Double_t a22;
  TVector3 d1;
  TVector3 d2;
  switch (sorting){
    case -1: //no sorting
      return kFALSE;

    // if clusters are NOT sorted by R, sort tracklets by R
    case 0:
    case 1:
    case 2:
    case 4:
      a1=t1->getFirstHit()->cluster()->pos().Perp();
      a12=t1->getLastHit()->cluster()->pos().Perp();
      if (a12<a1) a1=a12;

      a2=t2->getFirstHit()->cluster()->pos().Perp();
      a22=t2->getLastHit()->cluster()->pos().Perp();
      if (a22<a2) a2=a22;

      return a1<a2;

    // if clusters are sorted by R, sort tracklets by Z
    case 3:
    default:
      a1=t1->getFirstHit()->cluster()->pos().Z();
      a12=t1->getLastHit()->cluster()->pos().Z();
      if (a12<a1) a1=a12;

      a2=t2->getFirstHit()->cluster()->pos().Z();
      a22=t2->getLastHit()->cluster()->pos().Z();
      if (a22<a2) a2=a22;

      return a1<a2;
  }
}



