// =================================================
//  STEvent Class                          
//                                                  
//  Description:                                    
//    Container for a event data
//                                                  
//  Genie Jhang ( geniejhang@majimak.com )          
//  2014. 05. 19                                    
// =================================================

#include <iostream>
#include <iomanip>

#include "STEvent.hh"

ClassImp(STEvent);

STEvent::STEvent(Bool_t isClustered, Bool_t isTracked, Bool_t isChanged)
:TNamed("STEvent", "Event container")
{
  fEventID = -1;

  fIsClustered = isClustered;
  fIsTracked = isTracked;
  fIsChanged = isChanged;

  fIsGood = kFALSE;
}

STEvent::STEvent(STEvent *object)
:TNamed("STEvent", "Event container")
{
  fEventID = object -> GetEventID();

  fIsClustered = object -> IsClustered();
  fIsTracked = object -> IsTracked();
  fIsChanged = object -> IsChanged();

  fHitArray = *(object -> GetHitArray());

  if (IsClustered())
    fClusterArray = *(object -> GetClusterArray());

  fIsGood = object -> IsGood();
}

STEvent::~STEvent()
{
}

void STEvent::SetIsClustered(Bool_t value)   { fIsClustered = value; }
void STEvent::SetIsTracked(Bool_t value)     { fIsTracked = value; }
void STEvent::SetIsChanged(Bool_t value)     { fIsChanged = value; }

void STEvent::SetIsGood(Bool_t value)        { fIsGood = value; }

Bool_t STEvent::IsClustered()                { return fIsClustered; }
Bool_t STEvent::IsTracked()                  { return fIsTracked; }
Bool_t STEvent::IsChanged()                  { return fIsChanged; }

Bool_t STEvent::IsGood()                     { return fIsGood; }

Int_t STEvent::GetNumClusters()              { return fClusterArray.size(); }

// setters
void STEvent::SetEventID(Int_t evtid)                             { fEventID = evtid; } 
void STEvent::AddHit(STHit *hit)                                  { fHitArray.push_back(*hit); } 
void STEvent::SetHitArray(vector<STHit> *hitArray)                { fHitArray = *hitArray; } 
void STEvent::AddCluster(STHitCluster *cluster)                   { fClusterArray.push_back(*cluster); } 
void STEvent::SetClusterArray(vector<STHitCluster> *clusterArray) { fClusterArray = *clusterArray; }

// getters
Int_t STEvent::GetEventID() { return fEventID; }
Int_t STEvent::GetNumHits() { return fHitArray.size(); }

STHit *STEvent::GetHit(Int_t hitNo)
{
  return (hitNo < GetNumHits() ? &fHitArray[hitNo] : NULL);
}

STHit *STEvent::RemoveHit(Int_t hitNo)
{
  if (!(hitNo < GetNumHits()))
    return NULL;

  STHit *removedHit = new STHit(fHitArray[hitNo]);
  fHitArray.erase(fHitArray.begin() + hitNo);

  return removedHit;
}

vector<STHit> *STEvent::GetHitArray()
{
  return &fHitArray;
}

STHitCluster *STEvent::GetCluster(Int_t clusterNo)
{
  if (!(clusterNo < GetNumClusters()) || !IsClustered())
    return NULL;

  return &fClusterArray[clusterNo];
}

STHitCluster *STEvent::RemoveCluster(Int_t clusterNo)
{
  if (!(clusterNo < GetNumClusters()) || !IsClustered())
    return NULL;

  STHitCluster *removedCluster = new STHitCluster(fClusterArray[clusterNo]);
  fClusterArray.erase(fClusterArray.begin() + clusterNo);

  return removedCluster;
}

vector<STHitCluster> *STEvent::GetClusterArray()
{
  return &fClusterArray;
}
