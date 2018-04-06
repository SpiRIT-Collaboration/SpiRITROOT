#include "STRecoTrackCand.hh"
#include <iostream>

ClassImp(STRecoTrackCand)

STRecoTrackCand::STRecoTrackCand()
{
  Clear();
}

void STRecoTrackCand::Clear(Option_t *option)
{
  //fPID = STPID::kNon;
  fPIDProbability = 0;
  fIsEmbed = false;
  fMomentum.SetXYZ(0,0,0);
  fMomentumTargetPlane.SetXYZ(0,0,0);
  fPosTargetPlane.SetXYZ(0,0,0);

  fdEdxPointArray.clear();
}

void STRecoTrackCand::Print(Option_t *option) const
{
  cout << fPID << ": " << fMomentum.Mag() << " [MeV/c]" << endl;
}

void STRecoTrackCand::Copy(TObject *obj) const
{
  auto cand = (STRecoTrackCand *) obj;
  cand -> SetPID(fPID);
  cand -> SetPIDProbability(fPIDProbability);
  cand -> SetMomentum(fMomentum);
  cand -> SetMomentumTargetPlane(fMomentumTargetPlane);
  cand -> SetPosTargetPlane(fPosTargetPlane);
  cand -> SetGenfitTrack(fGenfitTrack);
}

Double_t STRecoTrackCand::GetdEdxWithCut(Double_t lowCut, Double_t highCut)
{
  vector<Double_t> dEdxArray;
  for (auto point : fdEdxPointArray)
    dEdxArray.push_back(point.fdE/point.fdx);

  sort(dEdxArray.begin(), dEdxArray.end());

  auto numUsedPoints = dEdxArray.size();
  Int_t idxLow = Int_t(numUsedPoints * lowCut);
  Int_t idxHigh = Int_t(numUsedPoints * highCut);

  numUsedPoints = idxHigh - idxLow;
  if (numUsedPoints < 1)
    return -1;

  Double_t dEdx = 0;
  for (Int_t idEdx = idxLow; idEdx < idxHigh; idEdx++)
    dEdx += dEdxArray[idEdx];
  dEdx = dEdx/numUsedPoints;

  return dEdx;
}
