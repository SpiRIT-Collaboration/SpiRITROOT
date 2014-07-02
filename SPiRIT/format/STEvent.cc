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
#include "STHit.hh"

ClassImp(STEvent);

STEvent::STEvent()
:TNamed("STEvent", "Event container")
{
  fEventID = -4;

  fIsClustered = 0;
  fIsTracked = 0;
}

STEvent::STEvent(STEvent *object)
:TNamed("STEvent", "Event container")
{
  fEventID = object -> GetEventID();

  fIsClustered = object -> IsClustered();
  fIsTracked = object -> IsTracked();

  fHitArray = object -> GetHitArray();
}

STEvent::~STEvent()
{
}

// setters
void STEvent::SetEventID(Int_t evtid)
{
  fEventID = evtid;
}

void STEvent::AddHit(STHit *hit)
{
  fHitArray.push_back(*hit);
}

// getters
Int_t STEvent::GetEventID()
{
  return fEventID;
}

Int_t STEvent::GetNumHits()
{
  return fHitArray.size();
}

STHit *STEvent::GetHit(Int_t hitNo)
{
  return (hitNo < GetNumHits() ? &fHitArray[hitNo] : NULL);
}

STHit *STEvent::RemoveHit(Int_t hitNo)
{
  if (!(hitNo < GetNumHits()))
    return 0;

  STHit *removedHit = new STHit(fHitArray[hitNo]);
  fHitArray.erase(fHitArray.begin() + hitNo);

  return removedHit;
}
