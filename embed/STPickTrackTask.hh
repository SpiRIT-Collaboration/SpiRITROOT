//---------------------------------------------------------------------
// Description:
//      Pick Track task class
//
// Author List:
//      Rensheng     Soochow Univ.
//----------------------------------------------------------------------

#ifndef _STPickTrackTask_H_
#define _STPickTrackTask_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"
#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

// SPiRITROOT classes
#include "STRawEvent.hh"
#include "STEmbedTrackContainer.hh"
#include "STVertex.hh"
#include "STPulse.hh"
#include "STPickTrackCut.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"

// STL
#include <vector>
#include "iostream"

using std::vector;
using namespace std;

class STPickTrackTask : public FairTask
{
public:
  // Constructor and Destructor
  STPickTrackTask();
  ~STPickTrackTask();
  // Operators
  
  // Getters
  STEmbedTrackContainer* GetEmbedContainer() { return EmbedContainer; }
  // Setters
  void SetStoreFileName(TString FileName_tem) { EmbedContainer->Init_StoreFile(FileName_tem.Data()); }
  void Set_LimitBoundary(double Pos_tem, double Amp_tem, double Angle_tem)
  {
    EmbedContainer->Set_LimitBoundary(Pos_tem, Amp_tem, Angle_tem);
  }
  void SetPIDProbability(double Prob_tem) { PIDProbability = Prob_tem; }
  void SetEventTrackNum(int MinTem, int MaxTem){ TrackNum_Min = MinTem; TrackNum_Max = MaxTem; }
  void SetEvtTag(string EvtTag_tem,string VertexTag_tem){ EvtTag = EvtTag_tem; VertexTag = VertexTag_tem; }
  void SetRunNo(int No_tem) {RunNo = No_tem;}
  void SetParticleCollected(int ParticleNum_tem, int* ParticleID_tem)
  {
    ParticleNum = ParticleNum_tem;
    for(int i=0;i<ParticleNum;i++) { ParticleID[i] = ParticleID_tem[i]; }
  }
  void SetMinCluster(int ClusterNum_tem) { MinClusterNum = ClusterNum_tem; }
  void SetIsStoreEachEvent(bool IsStoreEachEvt_tem) { IsStoreEachEvt = IsStoreEachEvt_tem; }
  void SetIsRawData_Recorded(bool IsRawData_Recorded_tem) { IsRawData_Recorded = IsRawData_Recorded_tem; }
  void SetIsPickTrackCutUsed( bool TagTem ) { IsPickTrackCutUsed = TagTem; }
  
  // Main methods
  virtual InitStatus Init();
  virtual void SetParContainers();
  virtual void Exec(Option_t *opt);
  void PickHits_onPad(STRawEvent* aRawEvent,STRecoTrack* aTrack);
  void PickHits_onStandardPulse(STRawEvent* aRawEvent,STRecoTrack* aTrack);

private:
  int RunNo;
  string EvtTag;
  string VertexTag;
  int ParticleNum;
  int ParticleID[10];
  double PIDProbability;
  int TrackNum_Min;
  int TrackNum_Max;
  int MinClusterNum;
  bool IsStoreEachEvt;
  bool IsRawData_Recorded;
  bool IsPickTrackCutUsed;
  
  int TrackID;

private:
  STEmbedTrackContainer* EmbedContainer;
  STPickTrackCut* aTrackCut;
  TClonesArray* fRawEventArray = nullptr;
  TClonesArray* fRecoTrackArray = nullptr;
  TClonesArray* fHitClusterArray = nullptr;
  TClonesArray *fVertexArray = nullptr;
  TClonesArray* fHitArray = nullptr;
  
  FairRootManager* fRootManager;
  STRawEvent* RawEvent_Dummy;
  STPulse* aPulse;
  TF1* StandardPulseFunction; //the spiritroot will produce the function with new(), so avoiding the memory leakage, we have to delete it each time.
  
  ClassDef(STPickTrackTask, 1);
};

#endif
