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
  STData(){}
  virtual ~STData(){}

  void ResetDefaultWithLength(int ntracks)
  {
    aoq = z = a = b = proja = projb = projx = projy = beamEnergy = beta = 0;
    multiplicity = 0;
    vaMultiplicity = 0;
    tpcVertex.SetXYZ(0,0,0);
    bdcVertex.SetXYZ(0,0,0);
    embedMom.SetXYZ(0,0,0);
    
    recoMom.clear();
    recoPosPOCA.clear();
    recoPosTargetPlane.clear();
    recodpoca.clear();
    recoNRowClusters.clear();
    recoNLayerClusters.clear();
    recoCharge.clear();
    recoEmbedTag.clear();
    recodedx.clear();

    vaMom.clear();
    vaPosPOCA.clear();
    vaPosTargetPlane.clear();
    vadpoca.clear();
    vaNRowClusters.clear();
    vaNLayerClusters.clear();
    vaCharge.clear();
    vaEmbedTag.clear();
    vadedx.clear();

    for(int i = 0; i < ntracks; ++i)
    {
      recoMom.emplace_back(0,0,0);
      recoPosPOCA.emplace_back(0,0,0);
      recoPosTargetPlane.emplace_back(0,0,0);
      recodpoca.emplace_back(0,0,0);
      recoNRowClusters.push_back(0);
      recoNLayerClusters.push_back(0);
      recoCharge.push_back(0);
      recoEmbedTag.push_back(false);
      recodedx.push_back(0);

      vaMom.emplace_back(0,0,0);
      vaPosPOCA.emplace_back(0,0,0);
      vaPosTargetPlane.emplace_back(0,0,0);
      vadpoca.emplace_back(0,0,0);
      vaNRowClusters.push_back(0);
      vaNLayerClusters.push_back(0);
      vaCharge.push_back(0);
      vaEmbedTag.push_back(false);
      vadedx.push_back(0);

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
  std::vector<TVector3> recoMom; 
  std::vector<TVector3> recoPosPOCA;  
  std::vector<TVector3> recoPosTargetPlane;  
  std::vector<TVector3> recodpoca; 
  std::vector<int> recoNRowClusters;  
  std::vector<int> recoNLayerClusters;  
  std::vector<int> recoCharge;  
  std::vector<bool> recoEmbedTag;  
  std::vector<double> recodedx;  

  // VA Data
  int vaMultiplicity;
  std::vector<TVector3> vaMom;  
  std::vector<TVector3> vaPosPOCA;  
  std::vector<TVector3> vaPosTargetPlane;  
  std::vector<TVector3> vadpoca; 
  std::vector<int> vaNRowClusters;  
  std::vector<int> vaNLayerClusters;  
  std::vector<int> vaCharge;  
  std::vector<bool> vaEmbedTag;  
  std::vector<double> vadedx;  

  TVector3 embedMom;
  ClassDef(STData, 1);
};

class STSmallOutputTask : public STRecoTask
{
public:
  STSmallOutputTask();
  virtual ~STSmallOutputTask();

  void SetOutputFile(const std::string& filename);
  void SetRun(int runID);
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
