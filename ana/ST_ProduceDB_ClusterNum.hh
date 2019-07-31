#ifndef ST_ProduceDB_ClusterNum_h
#define ST_ProduceDB_ClusterNum_h 1

#include "TObject.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "iostream"
#include "TMath.h"
#include "TF1.h"
#include "TCanvas.h"
#include "fstream"
#include "sstream"
#include "string.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TVector3.h"

using namespace std;
using namespace TMath;

class ST_ProduceDB_ClusterNum : public TObject
{
public:
  ST_ProduceDB_ClusterNum();
  ~ST_ProduceDB_ClusterNum();
  
public:
  int PositiveMomentumNum;
  string PositiveMomentumValue[1000];
  int NegativeMomentumNum;
  string NegativeMomentumValue[1000];
  
  //the below is for making which kinds of momentum will be in the DB.
  void Initial(string MomentumConfigFileNameTem);
  void ReadRawData(int ChargeValue,double MomentumValue,string RawDataFileName,TFile* f1_DB);
  void BuildDB(string FileName_Tem);
  
private:
  string MomentumConfigFileName;
  
  
  
  ClassDef(ST_ProduceDB_ClusterNum,1)
};

#endif
