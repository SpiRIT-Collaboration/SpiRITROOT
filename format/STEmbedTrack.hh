#ifndef STEMBEDTRACK
#define STEMBEDTRACK

#include <vector>
#include "STPID.hh"
#include "STRecoTrack.hh"
#include "TVector3.h"
#include "STMCTrack.h"
#include "TObject.h"
/**
 * SpiRIT Embedded Track Container
 *
 * All units in [mm], [ADC], [radian], [MeV].
 */

class STEmbedTrack : public TObject
{
  public:
  STEmbedTrack();
  ~STEmbedTrack();
  STEmbedTrack(STEmbedTrack *);

  void Clear();
  void SetFinalTrack(STRecoTrack *);
  void SetInitialTrack(STMCTrack *);

  TVector3 GetInitialMom(){return iMomentum;};
  TVector3 GetFinalMom(){return fMomentum;};
  TVector3 GetInitialVertex(){return iVertex;};
  TVector3 GetFinalVertex(){return fVertex;};
  STPID::PID GetInitialPID(){return iPID;};
  STPID::PID GetFinalPID(){return fPID;};

  void SetInitialPID(STPID::PID val){iPID = val;};
  void SetFinalPID(STPID::PID val){fPID = val;};
  void SetInitialMom(TVector3 val){ iMomentum = val;};
  void SetFinalMom(TVector3 val){fMomentum = val;};
  void SetInitialVertex(TVector3 val){iVertex = val;};
  void SetFinalVertex(TVector3 val){fVertex = val;};

private:

  Int_t helix_id;    //helix track ID
  Bool_t fIsEmbed;    //

  STPID::PID iPID;         //initial PID from MC
  STPID::PID fPID;         //final PID from software

  TVector3 fMomentum; //final momentum
  TVector3 iMomentum; //initial momentum

  TVector3 fVertex;   //fianl vertex
  TVector3 iVertex;   //initial vertex
  
  ClassDef(STEmbedTrack, 1)
};
#endif
