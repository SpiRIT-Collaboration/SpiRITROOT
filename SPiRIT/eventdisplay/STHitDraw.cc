// =================================================
//  STHit Class
//
//  Description:
//    Drawing STHit data
//
//  Original author: stockman (Apr 16, 2009)
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 07. 04
// ================================================= 

// SpiRITROOT classes
#include "STHitDraw.hh"

ClassImp(STHitDraw)

STHitDraw::STHitDraw()
{
}

STHitDraw::STHitDraw(const Char_t *name, Color_t color, Style_t style, Int_t verbose)
:STEventDraw(name, color, style, verbose)
{
}

TVector3 STHitDraw::GetVector(STHit &hit)
{
  return hit.GetPosition();
}
