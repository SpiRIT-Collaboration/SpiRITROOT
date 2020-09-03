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
    Bool_t IsGGCloseEvent();
    Bool_t IsBadEvent();


    void SetStatus(Int_t val);
    void SetIsEmptyEvent();
    void SetIsCollisionEvent();
    void SetIsActiveTargetEvent();
    void SetIsOffTargetEvent();
    void SetIsBeamEvent();
    void SetIsCosmicEvent();
    void SetIsGGCloseEvent();
    void SetIsBadEvent();

    void SetTbOffsets(Double_t *tbOffsets);
    Double_t *GetTbOffsets();
  
    void SetAoQ(Double_t val);
    Double_t GetAoQ();
    void SetZ(Double_t val);
    Double_t GetZ();

    void SetNRecoTrk(Int_t val);
    Int_t GetNRecoTrk();
    void SetNGoodRecoTrk(Int_t val);
    Int_t GetNGoodRecoTrk();
    void SetNVATrk(Int_t val);
    Int_t GetNVATrk();


  private:
    Int_t fEventID; ///< Original event ID from the run
  Double_t fAoQ;
  Double_t fZ;
  Int_t ntrk;
  Int_t ngtrk;
  Int_t nvtrk;

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

    Double_t fTbOffsets[12];

  ClassDef(STEventHeader, 1)
};

#endif
