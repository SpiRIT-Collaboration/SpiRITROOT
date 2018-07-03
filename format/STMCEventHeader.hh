#ifndef STMCEVENTHEADER
#define STMCEVENTHEADER

#include "TNamed.h"
#include "TVector3.h"
#include "TVector2.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "STMCTrack.h"

class STMCEventHeader : public TNamed
{
  public:
    STMCEventHeader();
    virtual ~STMCEventHeader();

    virtual void Clear(Option_t* opt= "");

    /** modifier **/
    void SetBeamVector(TLorentzVector v)   { fBeamVector = v; }
    void SetTargetVector(TLorentzVector v) { fTargetVector = v; }
    void SetB(Double_t b)              { fB = b; }
    void SetPrimaryVertex(TVector3 pv) { fPrimaryVertex = pv; }
    void SetBeamAngle(TVector2 ang)    { fBeamAngle = ang; }
    void SetReactionPlane(Double_t rp) { fReactionPlane = rp; }

    void SetPrimaryTracks(TClonesArray*);

    /** accessor **/
    Double_t GetBeamE(){ return (fBeamVector.E()-fBeamVector.M())/(fBeamVector.M()/931.494); }
    Double_t GetLorentzTransformFactor();

    Double_t GetB()             { return fB; }
    TVector3 GetPrimaryVertex() { return fPrimaryVertex; }
    TVector2 GetBeamAngle()     { return fBeamAngle; }
    Double_t GetReactionPlane() { return fReactionPlane; }

    TClonesArray* GetPrimaryTracks()     { return fPrimaryTrackArray; }
    STMCTrack* GetPrimaryTrack(Int_t id) { return (STMCTrack*)fPrimaryTrackArray->At(id); }


    /** rotation information **/
    TRotation GetRotationInRotatedFrame();
    TClonesArray* GetPrimaryTracksWithRotation();
    STMCTrack* GetPrimaryTrackWithRotation(Int_t id);


  private:
    TLorentzVector fBeamVector;
    TLorentzVector fTargetVector;

    Double_t fB;
    TVector3 fPrimaryVertex;
    TVector2 fBeamAngle;
    Double_t fReactionPlane;

    TClonesArray* fPrimaryTrackArray;

    ClassDef(STMCEventHeader,1);
};


#endif
