// =================================================
//  STStatic Class
//
//  Description:
//    Singleton for storing pointers
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2015. 09. 01
// =================================================

#include "STStatic.hh"

ClassImp(STStatic)

void *
STStatic::MakePointer(Long_t address)
{
  return (void *) address;
}
