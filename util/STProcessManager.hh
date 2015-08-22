#ifndef STPROCESSMANAGER
#define STPROCESSMANAGER

#include "TString.h"

class STProcessManager
{
  public :
    STProcessManager(TString name, 
                     Int_t   n, 
                     Int_t   d=100);
    ~STProcessManager() {};

    void PrintOut(Int_t i);
    void End();

   private :
     TString fProcessName;

     Int_t fNPoints;
     Int_t fDivision;
     Int_t fCount;
     Int_t fStep;
     Int_t fStepSize;

   ClassDef(STProcessManager, 1);
};

#endif
