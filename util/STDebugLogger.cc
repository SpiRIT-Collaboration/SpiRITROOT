#include "STDebugLogger.hh"

#define STLOG_COLOR_RED "\033[1m\033[32m"
#define STLOG_COLOR_RESET "\033[1m\033[32m"

ClassImp(STDebugLogger)

STDebugLogger* STDebugLogger::fInstance = NULL;
STDebugLogger* STDebugLogger::Create(TString name)
{
  if (fInstance == NULL)
    fInstance = new STDebugLogger(name);
  else 
    std::cout << "STDebugLogger already exist!" << std::endl;

  return fInstance;
}

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

  fOutFile = new TFile("st_debug_logger.root","RECREATE");
  fMaxBranchIdx = 0;
}

STDebugLogger::STDebugLogger(TString name)
{
  if (fInstance != NULL) throw;
  fInstance = this;

  name = name + ".root";
  fOutFile = new TFile(name,"RECREATE");
  fMaxBranchIdx = 0;
}

STDebugLogger::~STDebugLogger()
{
  if (fOutFile != NULL)
  {
    fOutFile -> Close();
    delete fOutFile;
  }
}

void 
STDebugLogger::Write()
{
  fOutFile -> cd();

  std::map<TString, TH1D*>::iterator it1 = fMapHist1.begin();
  while (it1 != fMapHist1.end()) 
  {
    std::cout << "STDebugLogger::Writing 1D Histogram " << it1 -> first << std::endl;
    it1 -> second -> Write();
    delete it1 -> second;
    it1++;
  }

  std::map<TString, TH2D*>::iterator it2 = fMapHist2.begin();
  while (it2 != fMapHist2.end()) 
  {
    std::cout << "STDebugLogger::Writing 2D Histogram " << it2 -> first << std::endl;
    it2 -> second -> Write();
    delete it2 -> second;
    it2++;
  }

  if (fOutFile != NULL) {
    fOutFile -> Close();
    delete fOutFile;
  }
}

void
STDebugLogger::FillHist1(TString name, Double_t val,
                         Int_t nbins, Double_t min, Double_t max)
{
  if (fMapHist1[name] == NULL)
  {
    fOutFile -> cd();
    TH1D* hist = new TH1D(name, "", nbins, min, max);
    fMapHist1[name] = hist;
  }
  TH1D* hist = fMapHist1[name];
  hist -> Fill(val);
}

void
STDebugLogger::FillHist1Step(TString name, Double_t val,
                             Int_t nbins, Double_t min, Double_t max)
{
  if (fMapHist1[name] == NULL)
  {
    fOutFile -> cd();
    TH1D* hist = new TH1D(name, "", nbins, min, max);
    fMapHist1[name] = hist;
  }
  TH1D* hist = fMapHist1[name];
  hist -> Fill(hist -> GetEntries(), val);
}

void
STDebugLogger::FillHist2(TString name, Double_t xval, Double_t yval,
                         Int_t xnbins, Double_t xmin, Double_t xmax,
                         Int_t ynbins, Double_t ymin, Double_t ymax)
{
  if (fMapHist2[name] == NULL)
  {
    fOutFile -> cd();
    TH2D* hist = new TH2D(name, "", xnbins, xmin, xmax, ynbins, ymin, ymax);
    fMapHist2[name] = hist;
  }

  TH2D* hist = fMapHist2[name];
  hist -> Fill(xval, yval);
}

void 
STDebugLogger::FillTree(TString name, Int_t nVal, Double_t *val, TString *bname)
{
  if (fMapTree[name] == NULL)
  {
    fOutFile -> cd();
    TTree* tree = new TTree(name, "");
    fMapTree[name] = tree;
    fMapBranchIdx[name] = fMaxBranchIdx;

    if (bname == NULL)
    {
      for (Int_t i=0; i<nVal; i++)
      {
        Double_t* branchVal = new Double_t;
        tree -> Branch(Form("v%d",i), &branchVal);
        fMapBranchVal.push_back(branchVal);

        fMaxBranchIdx++;
      }
    }
    else
    {
      for (Int_t i=0; i<nVal; i++)
      {
        Double_t* branchVal = new Double_t;
        tree -> Branch(bname[i], &branchVal);
        fMapBranchVal.push_back(branchVal);

        fMaxBranchIdx++;
      }
    }
  }

  Int_t idx = fMapBranchIdx[name];
  for (Int_t i=0; i<nVal; i++)
  {
    Double_t* valP = fMapBranchVal[idx+i];
    *valP = val[i];
  }
  fMapTree[name] -> Fill();
}

TFile* STDebugLogger::GetOutFile() { return fOutFile; }
TH1D*  STDebugLogger::GetHist1(TString name) { return fMapHist1[name] != NULL ? fMapHist1[name] : NULL; }
TH2D*  STDebugLogger::GetHist2(TString name) { return fMapHist2[name] != NULL ? fMapHist2[name] : NULL; }

void 
STDebugLogger::SetObject(TString name, TObject* object)
{
  if (fMapObject[name] == NULL)
    fMapObject[name] = object;
}

TObject* STDebugLogger::GetObject(TString name) { return fMapObject[name] != NULL ? fMapObject[name] : NULL; }


void STDebugLogger::Print(TString message)
{
  std::cout << "\033[1m\033[32m[STLOG]\033[0m "
            << message 
            << std::endl;
}

void STDebugLogger::Print(TString header, TString message)
{
  std::cout << "\033[1m\033[32m["
            << header
            << "]\033[0m "
            << message 
            << std::endl;
}
