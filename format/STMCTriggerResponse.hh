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
      void SetMult(Int_t mul)             { fMult = mul; }
      void SetMultWCT(Int_t mul)          { fMultWCT = mul; }
      void SetVetoMaxEdep(Double_t edep)  { fVetoMaxEdep = edep; }
      void SetVetoMaxZ(Int_t z)           { fVetoMaxZ = z; }
      void SetScintillatorHitArray(TClonesArray*);

      /** accessor **/
      Bool_t GetIsTrigger()     { return fIsTrigger; }
      Int_t GetMultiplicity()   { return fMult; }
      Int_t GetMultWCT()        { return fMultWCT; }
      Double_t GetVetoMaxEdep() { return fVetoMaxEdep; }
      Int_t GetVetoMaxZ()       { return fVetoMaxZ; }
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
