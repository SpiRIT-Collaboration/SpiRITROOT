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
#include "STHit.hh"

ClassImp(STHitDraw)

class TObject;

STHitDraw::STHitDraw()
{
}

STHitDraw::STHitDraw(const char* name, Int_t iVerbose)
:FairBoxSetDraw(name, iVerbose)
{
}

TVector3 STHitDraw::GetVector(TObject* object)
{
  return ((STHit *) object) -> GetPosition();
}
