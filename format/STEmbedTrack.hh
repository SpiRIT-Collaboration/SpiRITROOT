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
  void SetInitialTrack(STMCTrack *);
  
  STRecoTrack *GetFinalTrack(); //The most probable final track
  TVector3 GetInitialMom(){return iMomentum;};
  TVector3 GetInitialVertex(){return iVertex;};
  STPID::PID GetInitialPID(){return iPID;};
  
  void SetInitialPID(STPID::PID val){iPID = val;};
  void SetInitialMom(TVector3 val){ iMomentum = val;};
  void SetInitialVertex(TVector3 val){iVertex = val;};

  std::vector<STRecoTrack *> *GetRecoTrackArray() {return recotrack_ary;};
  void SetRecoTrackArray(std::vector<STRecoTrack *> *array) {recotrack_ary = array;};

private:

  Int_t helix_id;    //helix track ID
  Bool_t fIsEmbed;    //

  STPID::PID iPID;         //initial PID from MC
  TVector3 iMomentum; //initial momentum
  TVector3 iVertex;   //initial vertex
  
  //First element is the most probable track to be the final reco track
  //next are less probable but saved for study. Vector of size 0 means no track found.
  std::vector<STRecoTrack *> *recotrack_ary = new std::vector<STRecoTrack *>(0);
  
  ClassDef(STEmbedTrack, 1)
};
#endif
