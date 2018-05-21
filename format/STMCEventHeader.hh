#ifndef STMCEVENTHEADER
#define STMCEVENTHEADER

#include "TNamed.h"
#include "TVector3.h"
#include "TVector2.h"
#include "TClonesArray.h"
#include "STMCTrack.h"

class STMCEventHeader : public TNamed
{
   public:
     STMCEventHeader();
     virtual ~STMCEventHeader();

     virtual void Clear(Option_t* opt= "");

     /** modifier **/
     void SetBeamA(Int_t ba)   { fBeamA = ba; }
     void SetBeamZ(Int_t bz)   { fBeamZ = bz; }
     void SetBeamE(Int_t be)   { fBeamE = be; }
     void SetTargetA(Int_t ta) { fTargetA = ta; }
     void SetTargetZ(Int_t tz) { fTargetZ = tz; }
     void SetB(Double_t b)     { fB = b; }
     void SetPrimaryVertex(TVector3 pv) { fPrimaryVertex = pv; }
     void SetBeamAngle(TVector2 ang)    { fBeamAngle = ang; }
     void SetReactionPlane(Double_t rp) { fReactionPlane = rp; }

     void SetPrimaryTracks(TClonesArray*);

     /** accessor **/
     Int_t GetBeamA()   { return fBeamA; }
     Int_t GetBeamZ()   { return fBeamZ; }
     Double_t GetBeamE(){ return fBeamE; }
     Int_t GetTargetA() { return fTargetA; }
     Int_t GetTargetZ() { return fTargetZ; }
     Double_t GetLorentzTransformFactor();

     Double_t GetB()             { return fB; }
     TVector3 GetPrimaryVertex() { return fPrimaryVertex; }
     TVector2 GetBeamAngle()     { return fBeamAngle; }
     Double_t GetReactionPlane() { return fReactionPlane; }

     TClonesArray* GetPrimaryTracks()     { return fPrimaryTrackArray; }
     STMCTrack* GetPrimaryTrack(Int_t id) { return (STMCTrack*)fPrimaryTrackArray->At(id); }


   private:
     Int_t    fBeamA;
     Int_t    fBeamZ;
     Double_t fBeamE;
     Int_t    fTargetA;
     Int_t    fTargetZ;

     Double_t fB;
     TVector3 fPrimaryVertex;
     TVector2 fBeamAngle;
     Double_t fReactionPlane;

     TClonesArray* fPrimaryTrackArray;

   ClassDef(STMCEventHeader,1);
};

#endif
