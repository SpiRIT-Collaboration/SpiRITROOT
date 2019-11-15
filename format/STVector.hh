#ifndef STVECTOR_HH
#define STVECTOR_HH

#include "TObject.h"
#include "TVector3.h"

#include <vector>

struct STVectorF : public TObject
{
  STVectorF();
  std::vector<float> fElements;

  ClassDef(STVectorF, 1);
};


struct STVectorI : public TObject
{
  STVectorI();
  std::vector<int> fElements;

  ClassDef(STVectorI, 1);
};

struct STVectorVec3 : public TObject
{
  STVectorVec3();
  std::vector<TVector3> fElements;

  ClassDef(STVectorVec3, 1);
};


#endif 
