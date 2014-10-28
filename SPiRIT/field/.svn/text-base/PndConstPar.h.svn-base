/** PndConstPar.h
 ** @author M.Al-Turany
 ** @since 30.01.2007
 ** @version 1.0
 **
 ** Parameter set for the region between Solenoid and dipole. For the runtime database.
 **/


#ifndef PNDCONSTPAR_H
#define PNDCONSTPAR_H 1
#include "PndMapPar.h"
class FairParamList;

class PndConstPar : public PndMapPar
{

 public:

  
  /** Standard constructor  **/
  PndConstPar(const char* name, const char* title, const char* context);

/** default constructor  **/
  PndConstPar();
  
  /** Destructor **/
  ~PndConstPar();

  void putParams(FairParamList* list);


  /** Get parameters **/
  Bool_t getParams(FairParamList* list);


  /** Set parameters from FairField  **/
  void SetParameters(FairField* field);


  Double_t GetBx()        const { return fBx; }
  Double_t GetBy()        const { return fBy; }
  Double_t GetBz()        const { return fBz; }

 protected:

 /** Field values in [kG] **/
  Double_t fBx, fBy, fBz;

  ClassDef(PndConstPar,1);

};


#endif
