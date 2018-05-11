#include "STEveTask.hh"

ClassImp(STEveTask);

STEveTask::STEveObject 
STEveTask::GetEveObject(TString name)
{
  name.ToLower();

       if (name == "mc")           return kMC;
  else if (name == "digi")         return kDigi;
  else if (name == "hit")          return kHit;
  else if (name == "hitbox")       return kHitBox;
  else if (name == "cluster")      return kCluster;
  else if (name == "clusterbox")   return kClusterBox;
  else if (name == "riemanntrack") return kRiemannTrack;
  else if (name == "riemannhit")   return kRiemannHit;
  else if (name == "helix")        return kHelix;
  else if (name == "helixhit")     return kHelixHit;
  else if (name == "curve")        return kCurve;
  else if (name == "curvehit")     return kCurveHit;
  else if (name == "recotrack")    return kRecoTrack;
  else if (name == "recovertex")   return kRecoVertex;

  return kNon;
}

void STEveTask::DrawADCTask(Int_t row, Int_t layer)
{
  DrawADC(row, layer);

  TIter next(fTasks);
  STEveTask *task;
  while ((task = (STEveTask*)next()))
    task -> DrawADC(row, layer);
}

void STEveTask::UpdateWindowTbTask(Int_t start, Int_t end)
{
  UpdateWindowTb(start, end);

  TIter next(fTasks);
  STEveTask *task;
  while ((task = (STEveTask*)next()))
    task -> UpdateWindowTb(start, end);
}

void STEveTask::PushParametersTask()
{
  PushParameters();

  TIter next(fTasks);
  STEveTask *task;
  while ((task = (STEveTask*)next()))
    task -> PushParameters();
}

Int_t STEveTask::RnrEveObjectTask(TString name, Int_t option)
{
  Int_t rnr = RnrEveObject(name, option);
  
  if (rnr != -1)
    return rnr;

  TIter next(fTasks);
  STEveTask *task;
  while ((task = (STEveTask*)next()))
  {
    Int_t rnrTemp = task -> RnrEveObject(name, option);
    if (rnrTemp != -1)
      rnr = rnrTemp;
  }

  return rnr;
}

Int_t STEveTask::IsSetTask(TString name, Int_t option)
{
  Int_t set = IsSet(name, option);

  if (set != -1)
    return set;

  TIter next(fTasks);
  STEveTask *task;
  while ((task = (STEveTask*)next()))
  {
    Int_t setTemp = task -> IsSet(name, option);
    if (setTemp != -1)
      set = setTemp;
  }

  return set;
}
