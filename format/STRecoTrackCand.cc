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

  auto array = cand -> GetdEdxPointArray();
  for (auto dedx : fdEdxPointArray) {
    array -> push_back(dedx);
  }
}

Double_t STRecoTrackCand::GetdEdxWithCut(Double_t lowCut, Double_t highCut, Bool_t isContinuousHits, Int_t clusterSize)
{
  vector<STdEdxPoint> dEdxArray = fdEdxPointArray;

  sort(dEdxArray.begin(), dEdxArray.end(), [](STdEdxPoint a, STdEdxPoint b) -> Bool_t { return (a.fdE/a.fdx < b.fdE/b.fdx); });

  auto numUsedPoints = dEdxArray.size();
  Int_t idxLow = Int_t(numUsedPoints * lowCut);
  Int_t idxHigh = Int_t(numUsedPoints * highCut);

  numUsedPoints = idxHigh - idxLow;
  if (numUsedPoints < 1)
    return -1;

  Double_t dEdx = 0;
  for (Int_t idEdx = idxLow; idEdx < idxHigh; idEdx++) {
    if ((isContinuousHits & !dEdxArray[idEdx].fIsContinuousHits) || dEdxArray[idEdx].fClusterSize <= clusterSize) {
      numUsedPoints--;
      continue;
    }

    dEdx += dEdxArray[idEdx].fdE/dEdxArray[idEdx].fdx;
  }
  dEdx = dEdx/numUsedPoints;

  return dEdx;
}
