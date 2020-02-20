#include "TROOT.h"
#include "TLatex.h"
#include "STPIDProbTask.hh"
#include "STAnaParticleDB.hh"
#include "STVector.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// ROOT class
#include "TDatabasePDG.h"

ClassImp(STPIDProbTask);

STPIDProbTask::STPIDProbTask()
{ 
  fLogger = FairLogger::GetLogger(); 
  //fPDGLists = new STVectorI();

  for(int pdg : fSupportedPDG)
    fFlattenHist[pdg] = TH2F(TString::Format("Part%d", pdg), "PID;momentum;dedx", 60, 0, 4500,100, -100, 100);

  fPDGProb = new TClonesArray("STVectorF", fSupportedPDG.size());
  fSDFromLine = new TClonesArray("STVectorF", fSupportedPDG.size());
}

STPIDProbTask::~STPIDProbTask()
{}

InitStatus STPIDProbTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  auto namelist = ioMan -> GetBranchNameList();
  fData = (TClonesArray*) ioMan -> GetObject("STData");

  for(auto& pdg : fSupportedPDG)
    fMomPosteriorDistribution[pdg] = TH1F(TString::Format("PartPostMom%d", pdg), ";Momentum;", 60, 0, 4500);

  ioMan -> Register("Prob", "ST", fPDGProb, fIsPersistence);
  ioMan -> Register("SD", "ST", fSDFromLine, fIsPersistence);

  if(fPIDRegion) fLogger -> Info(MESSAGE_ORIGIN, "Found PID region cut. Will only consider data inside that cut");

  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto pdg = fSupportedPDG[i];
    fPDGProbMap[pdg] = static_cast<STVectorF*>(fPDGProb -> ConstructedAt(i));
    fSDFromLineMap[pdg] = static_cast<STVectorF*>(fSDFromLine -> ConstructedAt(i));
  }


  return kSUCCESS;
}

void
STPIDProbTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void STPIDProbTask::Exec(Option_t *opt)
{
  for(auto& ele : fPDGProbMap) ele.second -> fElements.clear();
  for(auto& ele : fSDFromLineMap) ele.second -> fElements.clear();

  auto data = (STData*) fData -> At(0);
  int npart = data -> multiplicity;

  for(int part = 0; part < npart; ++part)
  {
    int nClus = data -> vaNRowClusters[part] + data -> vaNLayerClusters[part];
    double poca = data ->recodpoca[part].Mag();
    const auto& mom = data -> vaMom[part];
    const auto& charge = data -> recoCharge[part];
    const auto& dedx = data -> vadedx[part];
    double momMag = mom.Mag();
    double mass = 0;

    std::map<int, double> NotNormPDGProb;
    double sumProb = 0;
    for(int pdg : fSupportedPDG)
    {
      double prob = 0;
      double sdFromLine = 0;
      if(fGLimit[pdg] -> IsInside(momMag, dedx))         
      {
        double yield = 1;
        // load the prior if it exists
        if(fMomPriorDistribution.size() == fSupportedPDG.size())
        {
          auto hist = fMomPriorDistribution[pdg];
          if(hist) yield = hist -> GetBinContent(hist -> GetXaxis() -> FindBin(momMag));
        }
        double mean =  fBBE[pdg]->Eval(momMag);
        double sd = fSigma[pdg]->Eval(momMag);
        sdFromLine = (dedx - mean)/sd;
        prob = yield*TMath::Gaus(dedx, mean, sd, true);
        if(std::isnan(prob)) prob = 0; // this is why we cannot use -Ofast
      }
      fSDFromLineMap[pdg] -> fElements.push_back(sdFromLine);
      sumProb += prob;
      NotNormPDGProb[pdg] = prob;
    }
    if(sumProb == 0) sumProb = 1;

    for(int pdg : fSupportedPDG)
    {
      double prob = NotNormPDGProb[pdg]/sumProb;
      fPDGProbMap[pdg] -> fElements.push_back(prob);
      // fill histogram for posterior
      if(prob > 0.5)
        if(nClus > fMinNClus && poca < fMaxDPOCA)
        {
          if(prob > 0.5)
            fMomPosteriorDistribution[pdg].Fill(momMag);
          for(auto& hist : fFlattenHist)
          {
            double expDedx = fBBE[hist.first]->Eval(momMag);
            hist.second.Fill(momMag, dedx - expDedx);
          }
        }
    }
  }
}

void STPIDProbTask::FinishTask()
{
  auto file = FairRootManager::Instance() -> GetOutFile();
  file -> cd();
  for(auto& hist : fFlattenHist)
    hist.second.Write();

  if(fIterateMeta)
  {
    fMetaFile -> cd();
    for(auto& hist : fMomPosteriorDistribution)
      hist.second.Write(TString::Format("Distribution%d", hist.first));
  }
}

void STPIDProbTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STPIDProbTask::SetMetaFile(const std::string& t_metafile, bool t_update) 
{
  fIterateMeta = t_update;
  if(fMetaFile) fMetaFile -> Close();
  fMetaFile = new TFile(t_metafile.c_str(), "update");
  for(int pdg : fSupportedPDG)
    fMomPriorDistribution[pdg] = (TH1F*) fMetaFile -> Get(TString::Format("Distribution%d", pdg));
}

void STPIDProbTask::SetPIDFitFile(const std::string& t_fitfile)
{
  if(fFitFile) fFitFile -> Close();
  fFitFile = new TFile(t_fitfile.c_str(), "update");
  for(int pdg : fSupportedPDG)
  {
    fBBE[pdg] = static_cast<TF1*>(fFitFile -> Get(TString::Format("BEE%d", pdg)));
    fBBE[pdg]->FixParameter(0, fBBE[pdg]->GetParameter(0));
    fBBE[pdg]->SetRange(0, 4500);
    fSigma[pdg] = static_cast<TF1*>(fFitFile -> Get(TString::Format("PIDSigma%d", pdg)));
    fSigma[pdg]->SetRange(0, 4500);
    fGLimit[pdg] = static_cast<TCutG*>(fFitFile -> Get(TString::Format("Limit%d", pdg)));
  }

  fPIDRegion = (TCutG*) fFitFile -> Get("PIDFullRegion");
}

void STPIDProbTask::FitPID(const std::string& anaFile, const std::string& fitFile)
{
  STAnaParticleDB::FillTDatabasePDG();
  TChain chain("cbmsim");
  chain.Add(anaFile.c_str());
  TFile output(fitFile.c_str(), "UPDATE");

  auto BBE = [](double *x, double *par)
  {
    double fitval;
    double beta = x[0]/sqrt(x[0]*x[0] + 931.5*par[0]);
    return (par[1] + par[6]*x[0])/pow(beta, par[4])*(par[2] - pow(beta, par[4]) - log(par[3] + pow(931.5*par[0]/x[0], par[5])));
  };

  TF1 tBBE("BBE", BBE, 0, 4500, 7);

  int nbinsx = 150;
  int nbinsy = 500;
  double momMin = 0;
  double momMax = 4000;
  double minDeDx = 0;
  double maxDeDx = 1500;

  TCanvas c1;
  TH2F PID("PID", "PID", nbinsx, momMin, momMax, nbinsy, minDeDx, maxDeDx);
  PID.Sumw2();
  tBBE.SetParameters(1,-800,13,190000,-80,6, 0);

  chain.Draw("STData[0].vadedx:STData[0].vaMom.Mag()>>PID", 
             "STData[0].vaNRowClusters + STData[0].vaNLayerClusters > 20 && STData[0].recodpoca.Mag() < 10", 
             "goff",100000);

  for(int pdg : std::vector<int>{2212,1000010020,1000010030,1000020030, 1000020040})
   {
    c1.cd();
    PID.Draw("colz");

    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    int A = int(particle -> Mass()/STAnaParticleDB::kAu2Gev);
    std::string pname(particle -> GetName());
    tBBE.FixParameter(0,A);

    bool drawBEE = true;
    auto BEEName = TString::Format("BEE%d", pdg);
    if(auto loadedBEE = (TF1*) output.Get(BEEName))
    {
      std::cout << "BEE line for particle " << pdg << " is found. Do you want to redraw it? (y/n)" << std::endl;
      c1.cd();
      loadedBEE -> SetRange(10, momMax);
      loadedBEE -> Draw("same");
      c1.Update();
      c1.Modified();

      std::string temp;
      std::cin >> temp;
      if(temp != "y") drawBEE = false;
      loadedBEE -> Delete();
    }

    if(drawBEE)
    {
      std::cout << "Please draw cut for " << pname << std::endl;
      TCutG *cutg = nullptr;
      while(!cutg) cutg = (TCutG*) c1.WaitPrimitive("CUTG", "[CUTG]");
      std::cout << "Please modify the cutg if needed. Press Ctrl-C to continue" << std::endl;
      c1.WaitPrimitive("temp", "[CUTG]");

      auto profile = STPIDProbTask::ProfileX(&PID, cutg);//PID.ProfileX("_px", 1, -1, "s [CUTG]");
      TH1F PIDMean("PIDMean", "Mean;momentum (MeV/c2);dEdX", nbinsx, momMin, momMax);
      TH1F PIDStd("PIDStd", "Std;momentum (MeV/c2);Std", nbinsx, momMin, momMax);
      cutg -> Delete();

      for(int i = 1; i < PID.GetNbinsX(); ++i)
      {
        PIDMean.SetBinContent(i, profile->GetBinContent(i));
        PIDStd.SetBinContent(i, profile->GetBinError(i));
      }

      tBBE.SetNpx(5000);
      std::string redraw = "y";
      while(redraw == "y")
      {
        PIDMean.Fit(&tBBE);
        PID.Draw("colz");
        c1.Update();
        PIDMean.Draw("PE same");
        std::cout << "Please inspect the result of the fit. You may adjust the parameters in the fit panel. Press Ctrl-C to continue" << std::endl;
        c1.WaitPrimitive("temp", "[CUTG]");

        // Obtain the tf1 after adjustment is made on fit panel
        auto newBBE = (TF1*) PIDMean.GetFunction("BBE");
        tBBE.SetParameters(newBBE->GetParameters());
        std::cout << "Please inspect the fitted lines. Do you want to refit? (y/n)" << std::endl;
        tBBE.Draw("L same");
        c1.Update();
        std::cin >> redraw;
      }

      output.cd(); 
      auto clonedMean = (TH1F*) PIDMean.Clone(TString::Format("PIDMean%d", pdg));
      clonedMean -> Write();

      auto clonedBBE = (TF1*) tBBE.Clone(BEEName);
      clonedBBE -> SetRange(0.1, 4500);
      clonedBBE -> Write();

      std::string fitname = "ErrorFit" + pname;
      TF1 fitError(fitname.c_str(), [](double *x, double *p){return p[0]/pow(x[0], p[1]) + p[2];}, 0, 4500, 3);
      {
        fitError.SetParameters(100,2,0);

        TCanvas c2("c2", "c2");
        PIDStd.GetYaxis()->SetLimits(0, PIDStd.GetMaximum());
        PIDStd.Fit(&fitError);
        PIDStd.GetYaxis()->SetRangeUser(0, PIDStd.GetMaximum());

        std::cout << "Please modify the fit with Fit Panel if needed. Then press Ctrl-C" << std::endl;
        c2.WaitPrimitive("temp", "[CUTG]");

        // Obtain the tf1 after adjustment is made on fit panel
        auto newfitError = (TF1*) PIDStd.GetFunction(fitname.c_str());
        fitError.SetParameters(newfitError->GetParameters());
      }
      output.cd();
      auto clonedFitError = (TF1*) fitError.Clone(TString::Format("PIDSigma%d", pdg));
      clonedFitError -> SetRange(0.1, 4500);
      clonedFitError -> Write();
    }

    // draw the acceptable range for the particle
    bool drawGLimit = true;
    auto GLimitName = TString::Format("Limit%d", pdg);
    if(auto loadedGLimit = (TCutG*) output.Get(GLimitName))
    {
      std::cout << "Particle limits for particle " << pdg << " is found. Do you want to redraw it? (y/n)" << std::endl;
      c1.cd();
      loadedGLimit -> Draw("L same");
      c1.Update();
      c1.Modified();

      std::string temp;
      std::cin >> temp;
      if(temp != "y") drawGLimit = false;
      loadedGLimit -> Delete();
    }
 
    if(drawGLimit)
    {
      c1.cd();
      std::cout << "Please draw the acceptable range for the particle. It can be lax because it only serves as a guild for iterative Bayes." << std::endl;
      if(auto obj = c1.GetListOfPrimitives()->FindObject("CUTG")) c1.GetListOfPrimitives()->Remove(obj);
      c1.Modified();

      TCutG *gLimit = nullptr;
      while(!gLimit) gLimit = (TCutG*) c1.WaitPrimitive("CUTG", "[CUTG]");
      auto clonedGLimit = gLimit -> Clone(GLimitName);
      output.cd();
      clonedGLimit -> Write();
    }
  }
}

TH1F* STPIDProbTask::ProfileX(TH2F* hist, TCutG* cutg)
{
  // don't know why I can't get desirable error from default TH2::ProfileX
  // reimplement to avoid any probelms
  int ny = hist -> GetNbinsY();
  double ymin = hist -> GetYaxis() -> GetBinLowEdge(1);
  double ymax = hist -> GetYaxis() -> GetBinUpEdge(ny);

  int nx = hist -> GetNbinsX();
  double xmin = hist -> GetXaxis() -> GetBinLowEdge(1);
  double xmax = hist -> GetXaxis() -> GetBinUpEdge(nx);

  TH1F *profile = new TH1F(TString::Format("%s_px", hist->GetName()), "", nx, xmin, xmax);
  for(int i = 1; i < hist->GetNbinsX(); ++i)
  {
    TH1F temp("temp", "temp", ny, ymin, ymax);
    for(int j = 1; j < ny; ++j)
      if(cutg -> IsInside(hist->GetXaxis()->GetBinCenter(i), hist->GetYaxis()->GetBinCenter(j)))
        temp.SetBinContent(j, hist->GetBinContent(i, j));
    TF1 gaus("gaus", "gaus");
    if(temp.GetEntries() > 0)
    {
      gaus.FixParameter(1, temp.GetMean());
      temp.Fit(&gaus, "Q");
    }
    else gaus.SetParameters(0,0,0);
    profile->SetBinContent(i, gaus.GetParameter(1));
    profile->SetBinError(i, gaus.GetParameter(2));
  }
  return profile;
}
