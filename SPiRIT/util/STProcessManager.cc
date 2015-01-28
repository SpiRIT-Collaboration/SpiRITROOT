#include "STProcessManager.hh"
#include <iostream>
#include <iomanip>

using namespace std;

ClassImp(STProcessManager);

STProcessManager::STProcessManager(TString name, Int_t n, Int_t d)
{
  fProcessName = name;

  fNPoints  = n;
  fDivision = d;

  fStep  = 100/fDivision;
  fCount = 0;
  fStepSize = fNPoints/fDivision;
}

void 
STProcessManager::PrintOut(Int_t i)
{
  if(i>fCount*fStepSize)
    cout << "\r[" << fProcessName << " ] Process : " 
         << setw(3) << (fCount++)*fStep << " %" << flush;
}

void 
STProcessManager::End() 
{ 
  cout << "\r[" << fProcessName << " ] COMPLETE            " << endl; 
}
