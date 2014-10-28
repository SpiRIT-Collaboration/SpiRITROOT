/** PndMultiFieldPar.h
 ** @author M.Al-Turany
 ** @since 30.01.2007
 ** @version 1.0
 **
 ** Parameter set for the region between Solenoid and dipole. For the runtime database.
 **/


#ifndef PNDMULTIFIELDPAR_H
#define PNDMULTIFIELDPAR_H 1
#include "PndMapPar.h"
#include "PndMultiField.h"

class FairParamList;

class PndMultiFieldPar : public PndMapPar
{

 public:

  
  /** Standard constructor  **/
  PndMultiFieldPar(const char* name, const char* title, const char* context);

/** default constructor  **/
  PndMultiFieldPar();
  
  
/** Destructor **/
  ~PndMultiFieldPar();

  void putParams(FairParamList* list);


  /** Get parameters **/
  Bool_t getParams(FairParamList* list);


  /** Set parameters from FairField  **/
  void SetParameters(FairField* field);
  
  TObjArray *GetParArray(){return fParArray; }
  

protected:

  
  TObjArray *fParArray;

  ClassDef(PndMultiFieldPar,1);

};


#endif
