#include "STGFBField.hh"

#include <iostream>

ClassImp(STGFBField)

STGFBField *STGFBField::fInstance = nullptr;

STGFBField::STGFBField(const char* mapName, const char* fileType)
{
  fFieldMap = new STFieldMap(mapName, fileType);
  fFieldMap -> SetPosition(0,-20.43,33);
  fFieldMap -> Init();
}

STGFBField *STGFBField::GetInstance(const char* mapName, const char* fileType)
{
  if (fInstance == nullptr)
    fInstance = new STGFBField(mapName, fileType);
  else
    std::cout << "== [STGFBField] Existing pointer is returned" << std::endl;

  return fInstance;
}

TVector3 STGFBField::get(const TVector3& pos) const
{
  auto pos_m = TVector3(pos.X(), pos.Y(), pos.Z());

  auto bx = fFieldMap -> GetBx(pos_m.X(), pos_m.Y(), pos_m.Z());
  auto by = fFieldMap -> GetBy(pos_m.X(), pos_m.Y(), pos_m.Z());
  auto bz = fFieldMap -> GetBz(pos_m.X(), pos_m.Y(), pos_m.Z());

  return TVector3(bx, by, bz);
}
