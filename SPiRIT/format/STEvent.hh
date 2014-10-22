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

using std::vector;

class STEvent : public TNamed {
  public:
    STEvent(Bool_t isClustered = kFALSE, Bool_t isTracked = kFALSE, Bool_t isChanged = kFALSE);
    STEvent(STEvent *object);
    ~STEvent();

    // setters
    void SetEventID(Int_t evtid);
    void AddHit(STHit *hit);
    void SetHitArray(vector<STHit> *hitArray);
    void AddCluster(STHitCluster *cluster);
    void SetClusterArray(vector<STHitCluster> *clusterArray);
//    void AddTrack(STTrack *track);
//    void SetTrackArray(vector<STTrack> &trackArray);

    void SetIsClustered(Bool_t value);
    void SetIsTracked(Bool_t value);
    void SetIsChanged(Bool_t value);

    void SetIsGood(Bool_t value);

    // getters
    Int_t GetEventID();

    Int_t GetNumHits();
    STHit *GetHit(Int_t hitNo);
    STHit *RemoveHit(Int_t hitNo);
    vector<STHit> *GetHitArray();

    Int_t GetNumClusters();
    STHitCluster *GetCluster(Int_t clusterNo);
    STHitCluster *RemoveCluster(Int_t clusterNo);
    vector<STHitCluster> *GetClusterArray();

//    Int_t GetNumTracks();
//    STTrack *GetTrack(Int_t trackNo);
//    STTrack *RemoveTrack(Int_t trackNo);
//    vector<STTrack> *GetTrackArray();

    Bool_t IsClustered();
    Bool_t IsTracked();
    Bool_t IsChanged();

    Bool_t IsGood();

  private:
    Bool_t fIsClustered;
    Bool_t fIsTracked;
    Bool_t fIsChanged;

    Bool_t fIsGood;

    Int_t fEventID;

    vector<STHit> fHitArray;
    vector<STHitCluster> fClusterArray;
//    vector<STTrack> fTrackArray;

  ClassDef(STEvent, 1);
};

#endif
