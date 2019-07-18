#include "STNeuLAND.hh"

ClassImp(STNeuLAND);

TVector3 STNeuLAND::GlobalPos(TVector3 localPos)
{
  TVector3 globalPos = localPos;
  globalPos.RotateY( fRotYNeuland_deg * TMath::DegToRad() );
  globalPos += TVector3(fOffxNeuland, fOffyNeuland, fOffzNeuland);
  return globalPos;
}



/// Convert global position to local position
TVector3 STNeuLAND::LocalPos(TVector3 globalPos)
{
  TVector3 localPos = globalPos;
  localPos -= TVector3(fOffxNeuland, fOffyNeuland, fOffzNeuland);
  localPos.RotateY( -fRotYNeuland_deg * TMath::DegToRad() );
  return localPos;
}



/// Get layer from bar-id
Int_t STNeuLAND::GetLayer(Int_t mcDetID)
{
  if (mcDetID < fFirstMCDetectorID && mcDetID >= fLastMCDetectorID ) {
    Int_t layer = Int_t((mcDetID-fFirstMCDetectorID)/fWidthBar);
    return layer;
  }
  return -1;
}



/// Get row from bar-id
Int_t STNeuLAND::GetRow(Int_t mcDetID)
{
  auto layer = GetLayer(mcDetID);
  if (layer < 0)
    return -1;
  Int_t row = mcDetID - layer*fWidthBar;
  return row;
}



/* Get (local)  center position from mc-det-id
 *
 * [neuland-bar-id]  = [mc-detector-id] - 4000
 * [neuland-veto-id] = [mc-detector-id] - 5000
 *
 * even 50s
 *   0 + 0-49 : layer 0, from bottom(0) to top(49)
 * 100 + 0-49 : layer 2, from bottom(0) to top(49)
 * 200 + 0-49 : layer 4, from bottom(0) to top(49)
 * 300 + 0-49 : layer 6, from bottom(0) to top(49)
 *
 * odd 50s
 *  50 + 0-49 : layer 1, from     -x(0) to  +x(49)
 * 150 + 0-49 : layer 3, from     -x(0) to  +x(49)
 * 250 + 0-49 : layer 5, from     -x(0) to  +x(49)
 * 250 + 0-49 : layer 7, from     -x(0) to  +x(49)
 */
TVector3 STNeuLAND::GetBarLocalPosition(Int_t mcDetID)
{
  auto layer = GetLayer(mcDetID);
  auto row = GetRow(mcDetID);

  TVector3 localPosition(0,0,0);
  if (layer > 0 && row > 0) {
    localPosition.SetZ((layer+.5)*fWidthBar);
    if (layer%2==0) localPosition.SetY(-fHalfLengthBar+(row+1)*fWidthBar);
    else            localPosition.SetX(-fHalfLengthBar+(row+1)*fWidthBar);
  }

  return localPosition;
}



Int_t STNeuLAND::IsAlongXNotY(Int_t mcDetID)
{
  if (GetLayer(mcDetID)%2==0)
    return false;
  return true;
}



/// Get (global) center position from mc-det-id
TVector3 STNeuLAND::GetBarGlobalPosition(Int_t mcDetID)
{
  return LocalPos(GetBarLocalPosition(mcDetID));
}
