#ifndef STCONFIGRUN_HH
#define STCONFIGRUN_HH

#include "TObject.h"
#include "TString.h"

#include <iostream>
#include <vector>
using namespace std;

class STConfigRun : public TObject
{
  private:
    static STConfigRun *fInstance;

    TString fSpiRITROOTPath;

  public:
    static STConfigRun* Instance();

    STConfigRun();
    virtual ~STConfigRun() {};

    TString SpiRITROOTVersion();

    TString RecoFileName(Int_t runNo, Int_t splitNo, TString path = "", TString tag = "");
    vector<TString> GetListOfRecoFiles(Int_t runNo, TString path = "", TString tag = "", TString version = "");

    TString ParameterFileName(TString name = "");
    TString GeometryFileName(TString name = "");

  ClassDef(STConfigRun, 1)
};

#endif
