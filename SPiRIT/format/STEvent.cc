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

  fHitsArray = new TClonesArray("STHit", 100);
}

STEvent::~STEvent()
{
  delete fHitsArray;
}

// setters
void STEvent::SetEventID(Int_t evtid)
{
  fEventID = evtid;
}

void STEvent::AddHit(STHit *hit)
{
  fHitsArray -> AddLast(hit);
}

// getters
Int_t STEvent::GetEventID()
{
  return fEventID;
}

Int_t STEvent::GetNumHits()
{
  return fHitsArray -> GetEntriesFast();
}

STHit *STEvent::GetHit(Int_t hitNo)
{
  return (hitNo < GetNumHits() ? (STHit *) fHitsArray -> At(hitNo) : 0);
}

STHit *STEvent::RemoveHit(Int_t hitNo)
{
  if (!(hitNo < GetNumHits()))
    return 0;

  STHit *removedHit = (STHit *) fHitsArray -> At(hitNo);
  fHitsArray -> RemoveAt(hitNo);

  return removedHit;
}
