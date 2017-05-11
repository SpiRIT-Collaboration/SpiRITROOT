#ifndef STEVENTHEADER
#define STEVENTHEADER

#include "TNamed.h"

class STEventHeader : public TNamed
{
  public:
    STEventHeader();
    virtual ~STEventHeader() {};

    virtual void Clear(Option_t *option = "");
    virtual void Print(Option_t *option = "") const;

    Int_t GetEventID();
    void SetEventID(Int_t val);

    Int_t GetStatus();
    Bool_t IsEmptyEvent();
    Bool_t IsCollisionEvent();
    Bool_t IsActiveTargetEvent();
    Bool_t IsOffTargetEvent();
    Bool_t IsBeamEvent();
    Bool_t IsCosmicEvent();
    Bool_t IsBadEvent();


    void SetStatus(Int_t val);
    void SetIsEmptyEvent();
    void SetIsCollisionEvent();
    void SetIsActiveTargetEvent();
    void SetIsOffTargetEvent();
    void SetIsBeamEvent();
    void SetIsCosmicEvent();
    void SetIsBadEvent();

    void SetTbOffsets(Double_t tbRef0, Double_t tbOffset1, Double_t tbOffset2, Double_t tbOffset3);

    Double_t GetTbRef0();
    Double_t GetTbOffset1();
    Double_t GetTbOffset2();
    Double_t GetTbOffset3();

  private:
    Int_t fEventID; ///< Original event ID from the run

    /** 
     * Status of the event 
     *
     * -1 : not set
     *  0 : empty
     *  1 : collision on target
     *  2 : active target
     *  3 : off target
     *  4 : beam
     *  5 : cosmic 
     *  6 : bad 
    */
    Int_t fStatus;

    Double_t fTbRef0;
    Double_t fTbOffset1;
    Double_t fTbOffset2;
    Double_t fTbOffset3;

  ClassDef(STEventHeader, 1)
};

#endif
