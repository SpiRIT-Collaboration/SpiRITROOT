#include "STDebugLogger.hh"

#define STLOG_COLOR_RED "\033[1m\033[32m"
#define STLOG_COLOR_RESET "\033[1m\033[32m"

using namespace std;

ClassImp(STDebugLogger)

STDebugLogger* STDebugLogger::fInstance = NULL;
STDebugLogger* STDebugLogger::Create(TString name)
{
  if (fInstance == NULL)
    fInstance = new STDebugLogger(name);
  else 
    cout << "STDebugLogger already exist!" << endl;

  return fInstance;
}

STDebugLogger* STDebugLogger::Instance() 
{
  if (fInstance == NULL)
    fInstance = new STDebugLogger();

  return fInstance;
}

STDebugLogger* STDebugLogger::InstanceX() 
{
  if (fInstance == NULL)
    fInstance = new STDebugLogger("");

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

  if (name != "")
  {
    name = name + ".root";
    fOutFile = new TFile(name,"RECREATE");
    fMaxBranchIdx = 0;
  }
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
  if (fOutFile == NULL)
    return;

  fOutFile -> cd();

  Int_t total_time = 0;

  std::map<TString, Int_t>::iterator itTimer = fTimeTotal.begin();
  while (itTimer != fTimeTotal.end()) 
  {
    total_time += itTimer -> second;
    itTimer++;
  }
  cout << "-- Total Time: " << total_time << " ms" << endl;

  itTimer = fTimeTotal.begin();
  while (itTimer != fTimeTotal.end()) 
  {
    cout << "-- Timer [" << itTimer -> first <<  "]: " << itTimer -> second 
              << " ms (" << std::fixed << std::setprecision(2) 
              << Double_t(itTimer -> second) / total_time * 100 << " %)" << endl;
    itTimer++;
  }
  cout << endl;

  std::map<TString, TH1D*>::iterator itHist1D = fMapHist1.begin();
  while (itHist1D != fMapHist1.end()) 
  {
    cout << itTimer -> second << endl;
    cout << "STDebugLogger::Writing 1D Histogram " << itHist1D -> first << endl;
    itHist1D -> second -> Write();
    delete itHist1D -> second;
    itHist1D++;
  }

  std::map<TString, TH2D*>::iterator itHist2D = fMapHist2.begin();
  while (itHist2D != fMapHist2.end()) 
  {
    cout << "STDebugLogger::Writing 2D Histogram " << itHist2D -> first << endl;
    itHist2D -> second -> Write();
    delete itHist2D -> second;
    itHist2D++;
  }

  std::map<TString, TTree*>::iterator itTree = fMapTree.begin();
  while (itTree != fMapTree.end()) 
  {
    cout << "STDebugLogger::Writing Tree " << itTree -> first << endl;
    itTree -> second -> Write();
    delete itTree -> second;
    itTree++;
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
        tree -> Branch(Form("v%d",i), branchVal);
        fMapBranchVal.push_back(branchVal);

        fMaxBranchIdx++;
      }
    }
    else
    {
      for (Int_t i=0; i<nVal; i++)
      {
        Double_t* branchVal = new Double_t;
        tree -> Branch(bname[i], branchVal);
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

TTree*
STDebugLogger::GetTree(TString name)
{
  return fMapTree[name];
}

void 
STDebugLogger::TimerStart(TString name)
{
  fTimeStamp[name] = std::chrono::high_resolution_clock::now();
}

void 
STDebugLogger::TimerStop(TString name) 
{ 
  st_time_t stamp = fTimeStamp[name];
  st_time_t now = std::chrono::high_resolution_clock::now();
  
  Int_t time_took = std::chrono::duration_cast<std::chrono::milliseconds>(now - stamp).count();
  fTimeTotal[name] = time_took + fTimeTotal[name];
}

void 
STDebugLogger::SetObject(TString name, TObject* object)
{
  //if (fMapObject[name] == NULL)
    fMapObject[name] = object;
}

TObject* STDebugLogger::GetObject(TString name) { return fMapObject[name] != NULL ? fMapObject[name] : NULL; }

void 
STDebugLogger::SetIntPar(TString name, Int_t val)
{
  //fItMapIntPar = fMapIntPar.find(name);
  //if (fItMapIntPar != fMapIntPar.end())
    fMapIntPar[name] = val;
}

Bool_t 
STDebugLogger::GetIntPar(TString name, Int_t &val) 
{ 
  fItMapIntPar = fMapIntPar.find(name);
  if (fItMapIntPar != fMapIntPar.end()) {
    val = fItMapIntPar -> second;
    return kTRUE;
  }

  return kFALSE;
}


void 
STDebugLogger::Print(TString message)
{
  cout << "\033[1m\033[32m[STLOG]\033[0m "
       << message 
       << endl;
}

void 
STDebugLogger::Print(TString header, TString message)
{
  cout << "\033[1m\033[32m["
       << header
       << "]\033[0m "
       << message 
       << endl;
}

TFile* STDebugLogger::GetOutFile() { return fOutFile; }
TH1D*  STDebugLogger::GetHist1(TString name) { return fMapHist1[name] != NULL ? fMapHist1[name] : NULL; }
TH2D*  STDebugLogger::GetHist2(TString name) { return fMapHist2[name] != NULL ? fMapHist2[name] : NULL; }
