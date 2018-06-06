#ifndef STPSA_RiseEdge_h
#define STPSA_RiseEdge_h 1

#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "iostream"
#include "TMath.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TArrow.h"
#include "fstream"
#include "TClonesArray.h"

#include "STRawEvent.hh"
#include "STRecoTrack.hh"
#include "STHitCluster.hh"
#include "STHit.hh"
#include "STPad.hh"
#include "STPulse.hh"

using namespace std;
using namespace TMath;

#define ADCNUM 270

class STPSA_RiseEdge : public TObject
{
public:
  STPSA_RiseEdge();
  ~STPSA_RiseEdge();
  
  void Analyze(STRawEvent* aRawEvent,TClonesArray* hitArray); // keep the function name and structure with Jung Woo
  void Show_1Pad(STRawEvent* aRawEvent,int LayerNum_tem,int RowNum_tem);
  void Show_9Pads(STRawEvent* aRawEvent,int LayerNum_tem,int RowNum_tem);
  void SetHitArray_STDPulse(TClonesArray* HitArray_tem) { HitArray = HitArray_tem; }
  void DrawHitSTPulse(int PadRowNum,int PadLayerNum);

public:
  void FindPeak_NoGGNoise(STPad* aPad);
  void AnaPeak_noGGNoise();
  void Draw(int PulseNum_tem, int* PulsePeakADC_tem);
  TCanvas* c1_PadADC_PeakCheck;
  TH1D* h1_PadADC_Value;
  TArrow* a1_PeakArrow[100];
  STPulse* aSTDPulse;
//  void AnaPeak_withGGNoise();
  
public: //used for storing the temperate results.
  double PadADCArray[ADCNUM];
/*
  int PadPulseNum;
  int IsPiledUp[100];
  int PiledUp_ADCIndex[100];
  int PulsePeakADCIndex[100]; // there won't have more than 100 peak in one pad...
  double PulsePeakADCValue[100];
*/
private:
  //From the stantard pulse, only for get the falling ADC when we calculate the piled-up pulse.
  TF1* Func1_STD_Pulse;
  int STDPulse_ArrayNum; //only record how many STDPulseFunction has been produced.
  TF1* f1_STDPulse[100];
  int ADCIndex_NoGGNoise; // here I should do a study several Event, get a distribution, then decide the Cut and the average of the GGNoise. Now, you can give a large safty number.
  double fThreshold ;
  double fMaxADC_Saturated;
  TClonesArray* HitArray;
  
  ClassDef(STPSA_RiseEdge,1)
};

#endif
