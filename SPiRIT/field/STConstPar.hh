/** STConstPar.h
 ** @author M.Al-Turany
 ** Modified by Prabi on 25/10/2014 for SPiRIT TPC
 ** Parameter set for the region between Solenoid and dipole. For the runtime database.
 **/


#ifndef STCONSTPAR_H
#define STCONSTPAR_H 1
#include "STMapPar.hh"
class FairParamList;

class STConstPar : public STMapPar
{

 public:

  
  /** Standard constructor  **/
  STConstPar(const char* name, const char* title, const char* context);

/** default constructor  **/
  STConstPar();
  
  /** Destructor **/
  ~STConstPar();

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

  ClassDef(STConstPar,1);

};


#endif
