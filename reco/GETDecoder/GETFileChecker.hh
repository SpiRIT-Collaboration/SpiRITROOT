// =================================================
//  GETFileChecker Class
// 
//  Description:
//    Check if the file exists or not
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2015. 09. 01
// =================================================

#ifndef GETFILECHECKER
#define GETFILECHECKER

#include "TString.h"

class GETFileChecker {
  public:
    static TString CheckFile(TString filename, Bool_t print = kTRUE);

  ClassDef(GETFileChecker, 1)
};

#endif
