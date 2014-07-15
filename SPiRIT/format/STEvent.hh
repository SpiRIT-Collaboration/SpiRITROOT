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
#include "STCluster.hh"
//#include "STTrack.hh"

class STEvent : public TNamed {
  public:
    STEvent(Bool_t isClustered = kFALSE, Bool_t isTracked = kFALSE);
    STEvent(STEvent *object);
    ~STEvent();

    // setters
    void SetEventID(Int_t evtid);
    void AddHit(STHit *hit);
    void AddCluster(STCluster *cluster);
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
    STCluster *GetCluster(Int_t clusterNo);
    STCluster *RemoveCluster(Int_t clusterNo);
    std::vector<STCluster> *GetClusterArray();

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
    std::vector<STCluster> fClusterArray;
//    std::vector<STTrack> fTrackArray;

  ClassDef(STEvent, 1);
};

#endif
