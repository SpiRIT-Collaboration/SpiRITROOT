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

#include "TObject.h"

class TString;

class GETFileChecker : public TObject {
  public:
    GETFileChecker() {};
    ~GETFileChecker() {};

    static TString CheckFile(TString filename);

  ClassDef(GETFileChecker, 1)
};

#endif
