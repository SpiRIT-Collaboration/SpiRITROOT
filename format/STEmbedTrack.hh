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

  Int_t GetNumRowClusters(){return num_row;};
  Int_t GetNumLayerClusters(){return num_layer;};
  Int_t GetNumRowClusters90(){return num_row90;};
  Int_t GetNumLayerClusters90(){return num_layer90;};
  
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

  Int_t num_row;    //number of row clusters 
  Int_t num_layer;  //number of layer clusters 

  Int_t num_row90;    //number of row clusters 
  Int_t num_layer90;  //number of layer clusters 

  STPID::PID iPID;         //initial PID from MC
  STPID::PID fPID;         //final PID from software

  TVector3 fMomentum; //final momentum
  TVector3 iMomentum; //initial momentum

  TVector3 fVertex;   //fianl vertex
  TVector3 iVertex;   //initial vertex
  
  ClassDef(STEmbedTrack, 1)
};
#endif
