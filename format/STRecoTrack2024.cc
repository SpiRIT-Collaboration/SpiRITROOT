#include "STRecoTrack2024.hh"

ClassImp(STRecoTrack2024);

STRecoTrack2024::STRecoTrack2024()
{
  Clear();
}

STRecoTrack2024::STRecoTrack2024(STRecoTrack2024 *recoTrack) : STRecoTrack::STRecoTrack(recoTrack)
{
  SetPosWindow(recoTrack -> GetPosWindow());
}

void STRecoTrack2024::Clear(Option_t *option)
{
  STRecoTrack::Clear();
  fPosWindow.SetXYZ(-9999,-9999,-9999);
}
