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
  fPosVtxCluster.SetXYZ(0,0,0);

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
  cand -> SetPriMomentum(fPriMomentum);
  cand -> SetMomentumTargetPlane(fMomentumTargetPlane);
  cand -> SetPosTargetPlane(fPosTargetPlane);
  cand -> SetPriPosTargetPlane(fPriPosTargetPlane);
  cand -> SetPosVtxCluster(fPosVtxCluster);
  cand -> SetGenfitTrack(fGenfitTrack);

  auto array = cand -> GetdEdxPointArray();
  for (auto dedx : fdEdxPointArray) {
    array -> push_back(dedx);
  }
}

Double_t STRecoTrackCand::GetdEdxWithCut(Double_t lowCut, Double_t highCut, Double_t fract_shadow, Bool_t isContinuousHits, Int_t clusterSize, Int_t numDiv, Int_t refDiv, Double_t cutMin, Double_t cutMax)
{
  vector<STdEdxPoint> tempdEdxArray;
  vector<STdEdxPoint> dEdxArray;

  if (numDiv > 0 && refDiv > -1) {
    auto divPoint = new Int_t[numDiv + 1];
    for (auto iDiv = 0; iDiv < numDiv + 1; iDiv++)
      divPoint[iDiv] = fdEdxPointArray.size()*(iDiv/Double_t(numDiv));

    auto meanValues = new Double_t[numDiv];
    for (auto iDiv = 0; iDiv < numDiv; iDiv++) {
      vector<Double_t> dedxs;
      for (auto iIndex = divPoint[iDiv]; iIndex < divPoint[iDiv + 1]; iIndex++)
        dedxs.push_back(fdEdxPointArray[iIndex].fdE/fdEdxPointArray[iIndex].fdx);
      meanValues[iDiv] = TMath::Mean(dedxs.begin(), dedxs.end());
    }

    for (auto iDiv = 0; iDiv < numDiv; iDiv++) {
      auto logRatio = TMath::Log2(meanValues[iDiv]/meanValues[refDiv]);
      if (cutMin < logRatio && logRatio < cutMax) {
        for (auto iIndex = divPoint[iDiv]; iIndex < divPoint[iDiv + 1]; iIndex++)
          tempdEdxArray.push_back(fdEdxPointArray[iIndex]);
      }
    }

    delete [] divPoint;
    delete [] meanValues;
  } else
    tempdEdxArray = fdEdxPointArray;

  for (auto point : tempdEdxArray)
    if( (1.*point.fNumShadowHits/point.fNumHits) <= fract_shadow)
      dEdxArray.push_back(point);

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

Int_t STRecoTrackCand::GetClustNum(Double_t fract_shadow)
{
  vector<Double_t> dEdxArray;
  for (auto point : fdEdxPointArray)
    {
      if( (1.*point.fNumShadowHits/point.fNumHits) <= fract_shadow)
	dEdxArray.push_back(point.fdE/point.fdx);
    }

  return dEdxArray.size();
}
