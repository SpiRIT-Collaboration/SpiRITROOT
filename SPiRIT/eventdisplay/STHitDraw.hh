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

#ifndef STHITDRAW_H
#define STHITDRAW_H

// SpiRITROOT classes
#include "STEventDraw.hh"
#include "STHit.hh"
#include "STHitCluster.hh"

// ROOT classes
#include "Rtypes.h"
#include "TVector3.h"

class STHitDraw: public STEventDraw
{
  public:
    STHitDraw();

    STHitDraw(const Char_t *name, Color_t color, Style_t style, Int_t verbose = 1);

  protected:
    TVector3 GetVector(STHit &hit);
    TVector3 GetVector(STHitCluster &cluster);

  ClassDef(STHitDraw, 1);
};

#endif
