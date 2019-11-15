#include "STData.hh"

void STData::ResetDefaultWithLength(int ntracks)
{
  aoq = z = a = b = proja = projb = projx = projy = beamEnergy = beta = 0;
  multiplicity = 0;
  vaMultiplicity = 0;
  tpcVertex.SetXYZ(0,0,0);
  bdcVertex.SetXYZ(0,0,0);
  embedMom.SetXYZ(0,0,0);
  
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
    recoMom.emplace_back(0,0,0);
    recoPosPOCA.emplace_back(0,0,0);
    recoPosTargetPlane.emplace_back(0,0,0);
    recodpoca.emplace_back(0,0,0);
    recoNRowClusters.push_back(0);
    recoNLayerClusters.push_back(0);
    recoCharge.push_back(0);
    recoEmbedTag.push_back(false);
    recodedx.push_back(0);

    vaMom.emplace_back(0,0,0);
    vaPosPOCA.emplace_back(0,0,0);
    vaPosTargetPlane.emplace_back(0,0,0);
    vadpoca.emplace_back(0,0,0);
    vaNRowClusters.push_back(0);
    vaNLayerClusters.push_back(0);
    vaCharge.push_back(0);
    vaEmbedTag.push_back(false);
    vadedx.push_back(0);

  }
}
