// ===========================================================
// STHoughSpaceLine Class
// Description:
//    Class that calculates the Hough Space of straight lines
//
//    Log: 
//         
// ===========================================================

#ifndef _STHoughSpaceLine_H_
#define _STHoughSpaceLine_H_

#include "TROOT.h"
#include "TObject.h"
#include "TVector3.h"
#include "TMath.h"
#include "TH2.h"
#include "TCanvas.h"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <iterator>
#include <utility>
#include <stdio.h>
#include <stdlib.h>


#include "STHit.hh"
#include "STHitCluster.hh"
#include "STEvent.hh"




class STHit;
class STHitCluster;
class STEvent;


class STHoughSpaceLine : public TObject
{

 public:
 STHoughSpaceLine();
 STHoughSpaceLine(STEvent *event);
 ~STHoughSpaceLine();

 void GetClusterSpace(STEvent *event);
 void Initialize();

 void SetGUIMode();
 void SetDebugMode(Int_t verbosity);

 TH2F *HistHough;
 TH2F *ClusterProjXZ;
 TCanvas *HC;

 Bool_t fGUIMode;
 Bool_t fDebugMode;

 

 //void SetClusteredEvent();

 private:
  
  Int_t fClustersNum;
  STHitCluster *SingleCluster;
  TVector3 fClusterPos;
  std::vector<Float_t> fDistO;
  std::vector<Float_t> fTheta; // Is this needed?

  std::map<Int_t,Int_t> HoughMap; //One or Two for each plane?
  Int_t EncodeMapKey(Float_t d,Float_t ang);
  std::map<Int_t,Int_t>::iterator fHoughSpace_enc;
  Int_t fMapKey;
  

  void SetXZSingle(Float_t x,Float_t z);
  void DrawHoughSpace();
  void ResetHoughSpace();

  ClassDef(STHoughSpaceLine,1);


};

#endif


