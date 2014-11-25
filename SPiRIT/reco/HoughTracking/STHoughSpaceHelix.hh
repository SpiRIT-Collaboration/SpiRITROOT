// =================================================================
// STHoughSpaceHelix Class
// Description:
//    Class that calculates the Hough Space of helical trajectories
//
//    Log: 
//         
// =================================================================

#ifndef _STHoughSpaceHelix_H_
#define _STHoughSpaceHelix_H_

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

class STHoughSpaceHelix : public TObject
{

	public:
 	 STHoughSpaceHelix();
	 ~STHoughSpaceHelix();

         void GetClusterSpace(STEvent *event);
         void Initialize();
	 void SetGUIMode();
         void SetDebugMode(Int_t verbosity);

	 Bool_t fGUIMode;
 	 Bool_t fDebugMode;

        TH2F *HistHoughXZ;
        TH2F *HelixProjXZ;
        TCanvas *HC;

	private:

        Int_t fClustersNum;
        STHitCluster *SingleCluster;
        TVector3 fClusterPos;

        std::map<Int_t,Int_t> HoughMap; //One or Two for each plane?
        Int_t EncodeMapKey(Float_t d,Float_t ang);
        std::map<Int_t,Int_t>::iterator fHoughSpace_enc;
        Int_t fMapKey;

        void SetSingle(Float_t x, Float_t y ,Float_t z);
  	void DrawHoughSpace();
  	void ResetHoughSpace();


ClassDef(STHoughSpaceHelix,1);

};

#endif




