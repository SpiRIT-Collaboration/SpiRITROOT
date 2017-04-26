#include "STCompiled.h"
#include "STConfigRun.hh"
#include "TSystem.h"
#include "dirent.h"

ClassImp(STConfigRun)

TString STConfigRun::SpiRITROOTPath()
{
  return TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
}

TString STConfigRun::SpiRITROOTVersion()
{
  return STVERSION;
}

TString STConfigRun::RecoFileName(Int_t runNo, Int_t splitNo, TString path, TString tag)
{
  if (path.IsNull())
    path = STConfigRun::SpiRITROOTPath() + "macros/data/";
  else
    path = path + "/";

  if (!tag.IsNull())
    tag = TString(".") + tag;

  TString name = path + Form("run%d_s%d",runNo,splitNo) + tag + ".reco." + TString(STVERSION) + ".root";

  return name;
}

vector<TString> STConfigRun::GetListOfRecoFiles(Int_t runNo, TString path, TString tag, TString version)
{
  vector<TString> list;

  if (path.IsNull())
    path = STConfigRun::SpiRITROOTPath() + "macros/data/";
  else
    path = path + "/";

  if (!version.IsNull())
    version = TString(".") + version;

  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir (path.Data())) != NULL) {
    while ((ent = readdir (dir)) != NULL) {

      TString name = ent -> d_name;
      if (name.Index(Form("run%4d",runNo)) != 0)
        continue;

      if (name.Index(tag+".reco") < 0)
        continue;

      if (name.Index(version+".root") >= 0) {
        cout << "+ " << path+name << endl;
        list.push_back(path+name);
        continue;
      }
    }
    closedir (dir);
  }

  return list;
}

TString STConfigRun::ParameterFileName(TString name)
{
  if (name.IsNull())
    name = "ST.parameters.Commissioning_201604.par";

  name = STConfigRun::SpiRITROOTPath() + "parameters/" + name;

  return name;
}

TString STConfigRun::GeometryFileName(TString name)
{
  if (name.IsNull())
    name = "geomSpiRIT.man.root";

  name = STConfigRun::SpiRITROOTPath() + "geometry/" + name;

  return name;
}
