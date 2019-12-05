#ifndef STDATA_HH
#define STDATA_HH

#include <vector>

#include "TObject.h"
#include "TVector3.h"

struct STData : public TObject
{
  STData(){}
  virtual ~STData(){}

  void ResetDefaultWithLength(int ntracks);
  
  // beam data
  double aoq, z, a, b, proja, projb, projx, projy, beamEnergy, beta;    
//  double aoq, z, a, b, proja, projb, projx, projy, beamEnergyTargetPlane, betaTargetPlane;

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

  double beamEnergyTargetPlane, betaTargetPlane;
  ClassDef(STData, 1);
};


#endif
