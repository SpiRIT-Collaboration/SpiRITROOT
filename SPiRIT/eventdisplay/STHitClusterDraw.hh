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

#ifndef STHITCLUSTERDRAW_H
#define STHITCLUSTERDRAW_H

// SpiRITROOT classes
#include "STEventDraw.hh"
#include "STHit.hh"
#include "STHitCluster.hh"

// ROOT classes
#include "Rtypes.h"
#include "TVector3.h"

class STHitClusterDraw: public STEventDraw
{
  public:
    STHitClusterDraw();

    STHitClusterDraw(const Char_t *name, Color_t color, Style_t style, Int_t verbose = 1);

  protected:
    TVector3 GetVector(STHit &hit);
    TVector3 GetVector(STHitCluster &cluster);

  ClassDef(STHitClusterDraw, 1);
};

#endif
