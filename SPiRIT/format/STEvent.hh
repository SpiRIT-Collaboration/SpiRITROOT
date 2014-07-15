// =================================================
//  STEvent Class                          
//                                                  
//  Description:                                    
//    Container for a event data (hits & tracks)
//                                                  
//  Genie Jhang ( geniejhang@majimak.com )          
//  2014. 05. 19                                    
// =================================================

#ifndef STEVENT_H
#define STEVENT_H

#include "TROOT.h"
#include "TObject.h"

#include <vector>

#include "STHit.hh"
#include "STHitCluster.hh"
//#include "STTrack.hh"

class STEvent : public TNamed {
  public:
    STEvent(Bool_t isClustered = kFALSE, Bool_t isTracked = kFALSE);
    STEvent(STEvent *object);
    ~STEvent();

    // setters
    void SetEventID(Int_t evtid);
    void AddHit(STHit *hit);
    void AddCluster(STHitCluster *cluster);
//    void AddTrack(STTrack *track);

    void SetIsClustered(Bool_t value);
    void SetIsTracked(Bool_t value);

    // getters
    Int_t GetEventID();

    Int_t GetNumHits();
    STHit *GetHit(Int_t hitNo);
    STHit *RemoveHit(Int_t hitNo);
    std::vector<STHit> *GetHitArray();

    Int_t GetNumClusters();
    STHitCluster *GetCluster(Int_t clusterNo);
    STHitCluster *RemoveCluster(Int_t clusterNo);
    std::vector<STHitCluster> *GetClusterArray();

//    Int_t GetNumTracks();
//    STTrack *GetTrack(Int_t trackNo);
//    STTrack *RemoveTrack(Int_t trackNo);
//    std::vector<STTrack> *GetTrackArray();

    Bool_t IsClustered();
    Bool_t IsTracked();

  private:
    Bool_t fIsClustered;
    Bool_t fIsTracked;

    Int_t fEventID;

    std::vector<STHit> fHitArray;
    std::vector<STHitCluster> fClusterArray;
//    std::vector<STTrack> fTrackArray;

  ClassDef(STEvent, 1);
};

#endif
