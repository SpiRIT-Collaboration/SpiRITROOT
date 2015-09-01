#include "STDebugLogger.hh"

ClassImp(STDebugLogger)

STDebugLogger* STDebugLogger::fInstance = NULL;

STDebugLogger* STDebugLogger::Instance() 
{
  if (fInstance == NULL)
    fInstance = new STDebugLogger();

  return fInstance;
}

STDebugLogger::STDebugLogger()
{
  if (fInstance != NULL) throw;
  fInstance = this;
  fOutFile = new TFile("spirit_debug.root","RECREATE");
}

STDebugLogger::~STDebugLogger()
{
  if(fOutFile != NULL)
  {
    fOutFile -> Close();
    delete fOutFile;
  }
}

void 
STDebugLogger::Write()
{
  fOutFile -> cd();
  std::map<TString, TH1D*>::iterator it = fMapHist.begin();

  while (it != fMapHist.end()) {
    std::cout << "STDebugLogger::Writing histogram " << it->first << std::endl;
    it -> second -> Write();
    delete it -> second;
    ++it;
  }

  if (fOutFile != NULL) {
    fOutFile->Close();
    delete fOutFile;
  }
}


void
STDebugLogger::Hist(TString  name,
                    Double_t val,
                    Int_t    nbins,
                    Double_t min,
                    Double_t max)
{
  if(fMapHist[name] == NULL)
  {
    fOutFile -> cd();
    TH1D* hist = new TH1D(name,"",nbins,min,max);
    fMapHist[name] = hist;
  }
  TH1D* hist = fMapHist[name];
  hist -> Fill(val);
}
