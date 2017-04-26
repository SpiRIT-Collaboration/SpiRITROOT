#ifndef STCONFIGRUN_HH
#define STCONFIGRUN_HH

#include "TObject.h"
#include "TString.h"

#include <iostream>
#include <vector>
using namespace std;

class STConfigRun : public TObject {
  public:
    static TString SpiRITROOTPath();
    static TString SpiRITROOTVersion();

    static TString RecoFileName(Int_t runNo, Int_t splitNo, TString path = "", TString tag = "");
    static vector<TString> GetListOfRecoFiles(Int_t runNo, TString path = "", TString tag = "", TString version = "");

    static TString ParameterFileName(TString name = "");
    static TString GeometryFileName(TString name = "");

    ClassDef(STConfigRun, 1)
};

#endif
