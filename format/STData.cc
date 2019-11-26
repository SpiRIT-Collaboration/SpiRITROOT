#include "STData.hh"

void STData::ResetDefaultWithLength(int ntracks)
{
  aoq = z = a = b = proja = projb = projx = projy = beamEnergy = beta = beamEnergyTargetPlane = betaTargetPlane = -9999;
  multiplicity = -9999;
  vaMultiplicity = -9999;
  tpcVertex.SetXYZ(-9999,-9999,-9999);
  bdcVertex.SetXYZ(-9999,-9999,-9999);
  embedMom.SetXYZ(-9999,-9999,-9999);
  
  recoMom.clear();
  recoPosPOCA.clear();
  recoPosTargetPlane.clear();
  recodpoca.clear();
  recoNRowClusters.clear();
  recoNLayerClusters.clear();
  recoCharge.clear();
  recoEmbedTag.clear();
  recodedx.clear();

  vaMom.clear();
  vaPosPOCA.clear();
  vaPosTargetPlane.clear();
  vadpoca.clear();
  vaNRowClusters.clear();
  vaNLayerClusters.clear();
  vaCharge.clear();
  vaEmbedTag.clear();
  vadedx.clear();

  for(int i = 0; i < ntracks; ++i)
  {
    recoMom.emplace_back(-9999,-9999,-9999);
    recoPosPOCA.emplace_back(-9999,-9999,-9999);
    recoPosTargetPlane.emplace_back(-9999,-9999,-9999);
    recodpoca.emplace_back(-9999,-9999,-9999);
    recoNRowClusters.push_back(-9999);
    recoNLayerClusters.push_back(-9999);
    recoCharge.push_back(-9999);
    recoEmbedTag.push_back(false);
    recodedx.push_back(-9999);

    vaMom.emplace_back(-9999,-9999,-9999);
    vaPosPOCA.emplace_back(-9999,-9999,-9999);
    vaPosTargetPlane.emplace_back(-9999,-9999,-9999);
    vadpoca.emplace_back(-9999,-9999,-9999);
    vaNRowClusters.push_back(-9999);
    vaNLayerClusters.push_back(-9999);
    vaCharge.push_back(-9999);
    vaEmbedTag.push_back(false);
    vadedx.push_back(-9999);

  }
}
