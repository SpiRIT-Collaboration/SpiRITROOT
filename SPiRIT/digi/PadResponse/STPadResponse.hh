#ifndef _STPADRESPONSE_H_
#define _STPADRESPONSE_H_

#include "TH2D.h"

#include "STDigiPar.hh"
#include "STPad.hh"

class STPadResponse
{
  public :
    STPadResponse(STDigiPar* par);
    ~STPadResponse();

    void SetPad(STPad* pad, Double_t, Double_t, Int_t);
    void WriteHistogram();

  private : 
    STDigiPar* fPar;
    TH2D* fPadPlane;

    Int_t fPadPlaneX;
    Int_t fPadPlaneZ;

    Int_t fPadSizeX;
    Int_t fPadSizeZ;

  ClassDef(STPadResponse, 1);
};

#endif
