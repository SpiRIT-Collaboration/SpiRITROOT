
/** STMapPar.h
 ** @author M.Al-Turany
 ** Modified by Prabi on 25/10/2014 for SPiRIT TPC
 **
 ** Parameter set for the region between Solenoid and dipole. For the runtime database.
 **/


#ifndef STMAPPAR_H
#define STMAPPAR_H 1


#include "FairField.h"
#include "FairParGenericSet.h"


class FairParamList;


class STMapPar : public FairParGenericSet
{

 public:

  
  /** Standard constructor  **/
  STMapPar(const char* name, const char* title, const char* context);

/** default constructor  **/
  STMapPar();
  
  /** Destructor **/
  ~STMapPar();


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
  void MapName(TString& name) { name = fMapName; }
  Double_t GetPositionX() const { return fPosX; }
  Double_t GetPositionY() const { return fPosY; }
  Double_t GetPositionZ() const { return fPosZ; }
  Double_t GetScale()     const { return fScale; }


protected:

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



  /** Field map name in case of field map **/
  TString fMapName;


  /** Field centre position for field map **/
  Double_t fPosX, fPosY, fPosZ;


  /** Scaling factor for field map **/
  Double_t fScale;


  ClassDef(STMapPar,1);

};


#endif
