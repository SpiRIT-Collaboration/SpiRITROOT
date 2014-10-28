/** PndFieldPar.h
 **
 ** Parameter set for the PND magnetic field. For the runtime database.
 **/

#ifndef PNDFIELDPAR_H
#define PNDFIELDPAR_H 1

#include "FairField.h"
#include "FairParGenericSet.h"

class FairParamList;

class PndFieldPar : public FairParGenericSet
{
 public:

  /** Standard constructor  **/
  PndFieldPar(const char* name, const char* title, const char* context);

  /** default constructor  **/
  PndFieldPar();
  
  /** Destructor **/
  ~PndFieldPar();

  /** Put parameters **/
  virtual void putParams(FairParamList* list);

  /** Get parameters **/
  virtual Bool_t getParams(FairParamList* list);

  /** Set parameters from FairField  **/
  void SetParameters(FairField* field);

  /** Accessors **/
  Int_t    GetType()      const { return fType; }
  Double_t GetXmin()      const { return fXmin; }
  Double_t GetXmax()      const { return fXmax; }
  Double_t GetYmin()      const { return fYmin; }
  Double_t GetYmax()      const { return fYmax; }
  Double_t GetZmin()      const { return fZmin; }
  Double_t GetZmax()      const { return fZmax; }
  Double_t GetBx()        const { return fBx; }
  Double_t GetBy()        const { return fBy; }
  Double_t GetBz()        const { return fBz; }
  void MapName(TString& name) { name = fMapName; }
  Double_t GetPositionX() const { return fPosX; }
  Double_t GetPositionY() const { return fPosY; }
  Double_t GetPositionZ() const { return fPosZ; }
  Double_t GetScale()     const { return fScale; }

 private:

  /** Field type
   ** 0 = constant field
   ** 1 = field map
   ** 2 = field map sym2 (symmetries in x and y)
   ** 3 = field map sym3 (symmetries in x, y and z)
   **/
  Int_t fType;

  /** Field limits in case of constant field **/
  Double_t fXmin, fXmax;
  Double_t fYmin, fYmax;
  Double_t fZmin, fZmax;

  /** Field values in case of constant field [kG] **/
  Double_t fBx, fBy, fBz;

  /** Field map name in case of field map **/
  TString fMapName;

  /** Field centre position for field map **/
  Double_t fPosX, fPosY, fPosZ;

  /** Scaling factor for field map **/
  Double_t fScale;

  ClassDef(PndFieldPar,1);
};


#endif
