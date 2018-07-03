#ifndef STMCTRIGGERRESPONSE
#define STMCTRIGGERRESPONSE

#include "TNamed.h"
#include "TClonesArray.h"

class STMCTriggerResponse : public TNamed
{
  public:
    STMCTriggerResponse();
    virtual ~STMCTriggerResponse();

    /** modifier **/
    void SetIsTrigger(Bool_t flag)      { fIsTrigger = flag; }
    void SetKyotoMult(Int_t mul)             { fMult = mul; }
    void SetKyotoMultWCT(Int_t mul)          { fMultWCT = mul; }
    void SetKatanaVetoMaxEdep(Double_t edep)  { fVetoMaxEdep = edep; }
    void SetKatanaVetoMaxZ(Int_t z)           { fVetoMaxZ = z; }
    void SetScintillatorHitArray(TClonesArray*);

    /** accessor **/
    Bool_t GetIsTrigger()     { return fIsTrigger; }
    Int_t GetKyotoMult()      { return fMult; }
    Int_t GetKyotoMultWCT()   { return fMultWCT; }
    Double_t GetKatanaVetoMaxEdep() { return fVetoMaxEdep; }
    Int_t GetKatanaVetoMaxZ()       { return fVetoMaxZ; }
    TClonesArray* GetScintillatorHitArray() { return fScintillatorHitArray; }

  private:
    Bool_t        fIsTrigger;
    Int_t         fMult;
    Int_t         fMultWCT;
    Double_t      fVetoMaxEdep;
    Int_t         fVetoMaxZ;
    TClonesArray* fScintillatorHitArray;

    ClassDef(STMCTriggerResponse,1);
};

#endif
