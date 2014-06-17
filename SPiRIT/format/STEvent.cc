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
{
  fEventID = -4;
  fNumHits = -4;

  memset(fHitsArray, 0, sizeof(fHitsArray));
}

STEvent::~STEvent()
{
  for (Int_t iHit = 0; iHit < fNumHits; iHit++)
    delete fHitsArray[iHit];
}

// setters
void STEvent::SetEventID(Int_t evtid)
{
  fEventID = evtid;
}

void STEvent::SetHit(STHit *pad)
{
  fHitsArray[fNumHits] = pad;
  fNumHits++;
}

// getters
Int_t STEvent::GetEventID()
{
  return fEventID;
}

Int_t STEvent::GetNumHits()
{
  return fNumHits;
}

STHit *STEvent::GetHit(Int_t padNo)
{
  return (padNo < fNumHits ? fHitsArray[padNo] : 0);
}
