// =================================================
//  STHitCluster Class
//
//  Description:
//    Drawing STHitCluster data
//
//  Original author: stockman (Apr 16, 2009)
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 08. 07
// ================================================= 

// SpiRITROOT classes
#include "STHitClusterDraw.hh"
#include "STHit.hh"

ClassImp(STHitClusterDraw);

STHitClusterDraw::STHitClusterDraw()
{
}

STHitClusterDraw::STHitClusterDraw(const Char_t *name, Color_t color, Style_t style, Int_t verbose)
:STEventDraw(name, color, style, verbose)
{
}

TVector3 STHitClusterDraw::GetVector(STHitCluster &cluster)
{
  return cluster.GetPosition();
}

TVector3 STHitClusterDraw::GetVector(STHit &hit)
{
  return TVector3(0., 0., 0.);
}
