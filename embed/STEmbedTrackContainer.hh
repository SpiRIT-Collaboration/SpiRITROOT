/*
This class is used to record and read track.

*/
#ifndef _STEMBEDTRACKCONTAINER_H_
#define _STEMBEDTRACKCONTAINER_H_

#include "iostream"
#include "fstream"
#include "string.h"
#include "TObject.h"
#include "TTree.h"
#include "TFile.h"
#include "TMath.h"
#include "STRawEvent.hh"
#include "STRecoTrack.hh"
#include "TClonesArray.h"
#include "STHit.hh"
#include "STVertex.hh"

using namespace std;
using namespace TMath;

#define MaxEvtNum	100000

class STEmbedTrackContainer : public TObject
{
public:
// Constructor and Destructor
  STEmbedTrackContainer();
  ~STEmbedTrackContainer();
    
  // Getters
  TFile* GetDataFile(){ return f1_data; }
  TTree* GetDataTree(){ return t1_data; }
  STRawEvent* GetRawEvent(){ return RawEvent; }
  // Setters
  void SetRunNo(int RunNo_tem){ RunNo = RunNo_tem;}
  // Main methods: the below is for storing;
  // define or update the TTree, and initialize the STRawEvent.
  bool Init_StoreFile(string FileName);
  bool Is_NeedStore(STRecoTrack* RecoTrack);
  void Set_LimitBoundary(double Pos_tem, double Amp_tem, double Angle_tem)
  {
    PosLimit = Pos_tem;
    MomentumAngleLimit = Angle_tem;
    MomentumAmpLimit = Amp_tem;
  }
  void AddTrack(STRawEvent* RawEvent_tem,STRecoTrack* RecoTrack_tem,STVertex* Vertex_tem);
  void SaveData(){ f1_data->cd(); t1_data->Write(); }
  
  // the below is for reading;
  //real event track, include noise.
  STRawEvent* const GetEvtTrack(int NeedPID,double* NeedPos, double* NeedMomentum,double* ResErr_Allowed);
//  STRawEvent* GetIdealTrack(double* Position, double* Momentum,double* ResErr); //the pulse is presented by the modified width pulse, try to decrease the effect of noise.
  
private:
  //this is used to store the RawEvent and RecoTrack;
  TFile* f1_data;
  TTree* t1_data;
  TClonesArray *fRawEventArray;
  TClonesArray *fRecoTrackArray;
  TClonesArray *fVertexArray;
  STRawEvent* RawEvent; //all stored STRawEvent should be assigned to this point. 
  STRecoTrack* RecoTrack; //all stored STRecoTrack should be assigned to this point.
  STVertex* Vertex;
  int RunNo; //record the RunNo of this track.
  
private:
  Int_t EvtNum;
  double PosLimit; //position, angle, amplitude
  double MomentumAngleLimit;
  double MomentumAmpLimit;
  
  double MomentumTheta[MaxEvtNum];
  double MomentumPhi[MaxEvtNum];
  double MomentumAmp[MaxEvtNum];
  double MomentumX[MaxEvtNum]; //the coordinate of Cartesian is for accelerating the calculation;
  double MomentumY[MaxEvtNum];
  double MomentumZ[MaxEvtNum];
  // the below position is the position when the track get through the center plane of the target.
  double PosX[MaxEvtNum];
  double PosY[MaxEvtNum];
  double PosZ[MaxEvtNum];
  int PID[MaxEvtNum];
  
  ClassDef(STEmbedTrackContainer, 1);
};

#endif
