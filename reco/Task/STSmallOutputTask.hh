#ifndef STSMALLOUTPUTTASK_HH
#define STSMALLOUTPUTTASK_HH

#include <vector>
#include <string>
#include <memory>

#include "STRecoTask.hh"
#include "STEmbedTrack.hh"
#include "STBeamInfo.hh"

#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"
#include "TClonesArray.h"

struct STData : public TObject
{
  STData()
  {
    recoMom = new TVector3[kMaxTracks]; 
    recoPosPOCA = new TVector3[kMaxTracks];
    recoPosTargetPlane = new TVector3[kMaxTracks];  
    recodpoca = new TVector3[kMaxTracks];
    recoNRowClusters = new int[kMaxTracks];  
    recoNLayerClusters = new int[kMaxTracks];  
    recoCharge = new int[kMaxTracks];  
    recoEmbedTag = new bool[kMaxTracks];  
    recodedx = new double[kMaxTracks];  

    vaMom = new TVector3[kMaxTracks]; 
    vaPosPOCA = new TVector3[kMaxTracks];
    vaPosTargetPlane = new TVector3[kMaxTracks];  
    vadpoca = new TVector3[kMaxTracks];
    vaNRowClusters = new int[kMaxTracks];  
    vaNLayerClusters = new int[kMaxTracks];  
    vaCharge = new int[kMaxTracks];  
    vaEmbedTag = new bool[kMaxTracks];  
    vadedx = new double[kMaxTracks];  
  }

  virtual ~STData()
  {
    delete[] recoMom; 
    delete[] recoPosPOCA;  
    delete[] recoPosTargetPlane;  
    delete[] recodpoca;
    delete[] recoNRowClusters;  
    delete[] recoNLayerClusters;  
    delete[] recoCharge;  
    delete[] recoEmbedTag;  
    delete[] recodedx;  

    delete[] vaMom; 
    delete[] vaPosPOCA;  
    delete[] vaPosTargetPlane;  
    delete[] vadpoca;
    delete[] vaNRowClusters;  
    delete[] vaNLayerClusters;  
    delete[] vaCharge;  
    delete[] vaEmbedTag;  
    delete[] vadedx;  
  }

  void Clear()
  {
    aoq = z = a = b = proja = projb = projx = projy = beamEnergy = beta = 0;
    recoMomVec.clear();
    multiplicity = 0;
    vaMultiplicity = 0;
    tpcVertex.SetXYZ(0,0,0);
    bdcVertex.SetXYZ(0,0,0);
    
    for(int i = 0; i < kMaxTracks; ++i)
    {
      recoMom[i].SetXYZ(0,0,0);
      recoPosPOCA[i].SetXYZ(0,0,0);
      recoPosTargetPlane[i].SetXYZ(0,0,0);
      recodpoca[i].SetXYZ(0,0,0);
      recoNRowClusters[i] = 0;
      recoNLayerClusters[i] = 0;
      recoCharge[i]= 0;
      recoEmbedTag[i] = false;
      recodedx[i] = 0;

      vaMom[i].SetXYZ(0,0,0);
      vaPosPOCA[i].SetXYZ(0,0,0);
      vaPosTargetPlane[i].SetXYZ(0,0,0);
      vadpoca[i].SetXYZ(0,0,0);
      vaNRowClusters[i] = 0;
      vaNLayerClusters[i] = 0;
      vaCharge[i]= 0;
      vaEmbedTag[i] = false;
      vadedx[i] = 0;
    }
  }
  // beam data
  double aoq, z, a, b, proja, projb, projx, projy, beamEnergy, beta;    

  //Vertex
  TVector3 tpcVertex; 
  TVector3 bdcVertex; 

  static const int kMaxTracks = 100; // assume that max multiplicity in the experiment is 100
  int multiplicity;
  // Reco Data
  std::vector<TVector3> recoMomVec;
  TVector3 *recoMom; //[multiplicity]
  TVector3 *recoPosPOCA; //[multiplicity] 
  TVector3 *recoPosTargetPlane; //[multiplicity] 
  TVector3 *recodpoca; //[multiplicity]
  int *recoNRowClusters; //[multiplicity] 
  int *recoNLayerClusters; //[multiplicity] 
  int *recoCharge; //[multiplicity] 
  bool *recoEmbedTag; //[multiplicity] 
  double *recodedx; //[multiplicity] 

  // VA Data
  int vaMultiplicity;
  TVector3 *vaMom; //[multiplicity] 
  TVector3 *vaPosPOCA; //[multiplicity] 
  TVector3 *vaPosTargetPlane; //[multiplicity] 
  TVector3 *vadpoca; //[multiplicity]
  int *vaNRowClusters; //[multiplicity] 
  int *vaNLayerClusters; //[multiplicity] 
  int *vaCharge; //[multiplicity] 
  bool *vaEmbedTag; //[multiplicity] 
  double *vadedx; //[multiplicity] 

  ClassDef(STData, 1);
};

class STSmallOutputTask : public STRecoTask
{
public:
  STSmallOutputTask();
  virtual ~STSmallOutputTask();

  void SetOutputFile(const std::string& filename, const std::string& treename="proton");
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void FinishTask();
protected:
  TClonesArray *fSTRecoTrack = nullptr;
  TClonesArray *fVATracks = nullptr;
  TClonesArray *fSTVertex = nullptr;
  TClonesArray *fBDCVertex = nullptr;
  TClonesArray *fSTEmbedTrack = nullptr;
  STBeamInfo *fBeamInfo = nullptr;

  STData fData; //!< the class of data itself 
  std::unique_ptr<TFile> fSmallOutput_; //!< File where the tree is stored
  TTree *fSmallTree_; //!< Tree itself
  int fEventID;
  int fRunID;
  int fEventType;

  ClassDef(STSmallOutputTask,1);
};

#endif
