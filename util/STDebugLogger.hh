#ifndef STDEBUGLOGGER
#define STDEBUGLOGGER

#define DEBUGRIEMANNCUTS

#include "TFile.h"
#include "TH1D.h"

#include <map> 
#include <vector>
#include <iostream>

class STDebugLogger
{
  public:
    static STDebugLogger* Instance();
    STDebugLogger();
    ~STDebugLogger();

    void Write();
    void Hist(TString  name,
              Double_t val,
              Int_t    nbins = 100,
              Double_t min = 0, 
              Double_t max = 100);

  private:
    TFile* fOutFile;
    std::map<TString, TH1D*> fMapHist;

    static STDebugLogger* fInstance;

  ClassDef(STDebugLogger,0)
};

#endif
