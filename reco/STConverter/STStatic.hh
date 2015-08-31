// =================================================
//  STStatic Class
//
//  Description:
//    Singleton for storing pointers
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2015. 09. 01
// =================================================

#ifndef STSTATIC
#define STSTATIC

#include "TObject.h"

class STStatic : public TObject {
  public:
    STStatic() {};
    ~STStatic() {};

    static void *MakePointer(Long_t address);

  ClassDef(STStatic, 1)
};

#endif
