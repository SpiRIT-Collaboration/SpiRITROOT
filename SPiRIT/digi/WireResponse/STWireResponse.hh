#ifndef _STWIRERESPONSE_H_
#define _STWIRERESPONSE_H_

#include "TH2D.h"

class STWireResponse
{
  public :
    STWireResponse();
    ~STWireResponse();

  private :
    Bool_t fWriteHistogram;

  ClassDef(STWireResponse, 1);
};

#endif
