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

  Double_t total_time;

  std::map<TString, TStopwatch*>::iterator itTimer = fTimer.begin();
  while (itTimer != fTimer.end()) 
  {
    total_time += itTimer -> second -> RealTime();
    itTimer++;
  }
  itTimer = fTimer.begin();
  while (itTimer != fTimer.end()) 
  {
    std::cout << std::endl;
    std::cout << "STDebugLogger::Summary of Timer " 
              << itTimer -> first 
              << ", (" << itTimer -> second -> RealTime()/total_time * 100 << " %)" 
              << std::endl;
    itTimer -> second -> Print("m");
    itTimer++;
  }
  std::cout << std::endl;

  std::map<TString, TH1D*>::iterator itHist1D = fMapHist1.begin();
  while (itHist1D != fMapHist1.end()) 
  {
    std::cout << "STDebugLogger::Writing 1D Histogram " << itHist1D -> first << std::endl;
    itHist1D -> second -> Write();
    delete itHist1D -> second;
    itHist1D++;
  }

  std::map<TString, TH2D*>::iterator tiHist2D = fMapHist2.begin();
  while (tiHist2D != fMapHist2.end()) 
  {
    std::cout << "STDebugLogger::Writing 2D Histogram " << tiHist2D -> first << std::endl;
    tiHist2D -> second -> Write();
    delete tiHist2D -> second;
    tiHist2D++;
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

void STDebugLogger::TimerStart(TString name)
{
  if (fTimer[name] == NULL)
  {
    TStopwatch *timer = new TStopwatch();
    fTimer[name] = timer;
  }
  TStopwatch *timer = fTimer[name];
  timer -> Start(kFALSE);
}

void STDebugLogger::TimerStop(TString name) 
{ 
  TStopwatch *timer = fTimer[name];
  timer -> Stop(); 
}

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

TFile* STDebugLogger::GetOutFile() { return fOutFile; }
TH1D*  STDebugLogger::GetHist1(TString name) { return fMapHist1[name] != NULL ? fMapHist1[name] : NULL; }
TH2D*  STDebugLogger::GetHist2(TString name) { return fMapHist2[name] != NULL ? fMapHist2[name] : NULL; }
