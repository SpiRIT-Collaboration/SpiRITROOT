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

#include "STHit.hh"
//#include "STTrack.hh"

class STEvent : public TObject {
  public:
    STEvent();
    ~STEvent();

    // setters
    void SetEventID(Int_t evtid);
    void SetHit(STHit *hit);

    // getters
    Int_t GetEventID();
    Int_t GetNumHits();
    STHit *GetHit(Int_t hitNo);

  private:
    Int_t fEventID;
    Int_t fNumHits;
    STHit *fHitsArray[40000];

  ClassDef(STEvent, 1);
};

#endif
