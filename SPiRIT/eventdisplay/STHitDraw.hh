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

// FairRoot classes
#include "FairBoxSetDraw.h"

// ROOT classes
#include "Rtypes.h"
#include "TVector3.h"

class TObject;

class STHitDraw: public FairBoxSetDraw
{
  public:
    STHitDraw();

    /** Standard constructor
      *@param name        Name of task
      *@param iVerbose    Verbosity level
      **/
    STHitDraw(const char* name, Int_t iVerbose = 1);

  protected:

    TVector3 GetVector(TObject* object);

    ClassDef(STHitDraw, 1);
};

#endif
