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
#include "TTimer.h"
#include "TDatabasePDG.h"
#include "TGInputDialog.h"
#include "TButton.h"
#include "TGMsgBox.h"
#include "TDialogCanvas.h"

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

auto has_suffix = [](const std::string &str, const std::string &suffix)
  {
      return str.size() >= suffix.size() &&
             str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  };


void STPIDProbTask::FitPID(const std::string& anaFile, const std::string& fitFile)
{
  STAnaParticleDB::FillTDatabasePDG();
  std::string treeName = "cbmsim";

  TChain chain("cbmsim");
  std::string anaFileWithSuffix = anaFile;
  if(!has_suffix(anaFile, ".root")) anaFileWithSuffix += ".root";
  // the tree name could be cbmsim or spirit
  // need to add both
  {
    chain.Add((anaFileWithSuffix + "/spirit").c_str());
    chain.Add((anaFileWithSuffix + "/cbmsim").c_str());
    // suppress all the error due to not able to read from one of those trees
    auto origVerbosity = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kFatal;
    if(chain.GetEntries() == 0) throw std::runtime_error("No entries is being read from the file!");
    gErrorIgnoreLevel = origVerbosity;
  }

  TFile output(fitFile.c_str(), "UPDATE");
  if(output.IsZombie()) throw std::runtime_error("The designated output fit file " + fitFile + " is not writable!");

  auto BBE = [](double *x, double *par)
  {
    double fitval;
    double beta = x[0]/sqrt(x[0]*x[0] + 931.5*par[0]);
    return (par[1] + par[6]*x[0])/pow(beta, par[4])*(par[2] - pow(beta, par[4]) - log(par[3] + pow(931.5*par[0]/x[0], par[5])));
  };

  TF1 tBBE("BBE", BBE, 0, 4500, 7);

  int nbinsx = 150;
  int nbinsy = 1000;
  double momMin = 0;
  double momMax = 4000;
  double minDeDx = 0;
  double maxDeDx = 5000;

  TCanvas c1;
  TH2F PID("PID", "PID", nbinsx, momMin, momMax, nbinsy, minDeDx, maxDeDx);
  PID.Sumw2();
  tBBE.SetParameters(1,-800,13,190000,-80,6, 0);

  chain.SetAlias("phi", "(vaMom.Phi() > 0)? vaMom.Phi()*TMath::RadToDeg() : vaMom.Phi()*TMath::RadToDeg() + 360");
  chain.Draw("vadedx:vaMom.Mag()>>PID", 
             "vaNRowClusters + vaNLayerClusters > 15 && recodpoca.Mag() < 15 && (fabs(vaMom.Phi()*TMath::RadToDeg()) < 30 || fabs(phi - 180) < 30) ", 
             "goff",100000);

  for(const int pdg : STAnaParticleDB::SupportedPDG)
   {
    c1.cd();
    c1.SetLogz();
    PID.Draw("colz");

    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    int A = int(particle -> Mass()/STAnaParticleDB::kAu2Gev);
    std::string pname(particle -> GetName());
    tBBE.FixParameter(0,A);

    // required output
    auto tightCutName = TString::Format("TightCut%d", pdg);
    auto tightCut = (TCutG*) output.Get(tightCutName);;
    auto BEEName = TString::Format("BEE%d", pdg);
    auto BEE = (TF1*) output.Get(BEEName);
    auto PIDSigmaName = TString::Format("PIDSigma%d", pdg);
    auto PIDSigma = (TF1*) output.Get(PIDSigmaName);
    auto GLimitName = TString::Format("Limit%d", pdg);
    auto *GLimit = (TCutG*) output.Get(GLimitName);;

    auto AskRedrawIfExist = [](TObject *obj, const char *drawOpt, TCanvas *c2, const char *question)
    {
      if(obj)
      {
        if(c2)
        {
          c2 -> cd();
          obj -> Draw(drawOpt);
          c2 -> Update();
          c2 -> Modified();
        }
        int response;
        new TGMsgBox(
          gClient->GetDefaultRoot(),
          gClient->GetDefaultRoot(),
          "",
          question,
          kMBIconQuestion,
          kMBYes+kMBNo,
          &response,  0, 0);
        if(response == kMBYes) 
        {
          obj -> Delete();
          obj = nullptr;
        }
      }
      return obj;
    };

    // need to redraw
    tightCut = (TCutG*) AskRedrawIfExist(tightCut, "l same", &c1, TString::Format("Tight cut for %s is found. Redraw?", pname.c_str()));
    
    if(!tightCut)
    {
      {
        TPaveText text(0.5, 0.7, .85, .8, "brNDC");
        text.AddText(TString::Format("Please draw cut for %s", pname.c_str()));
        text.Draw();
        while(!tightCut) tightCut = (TCutG*) c1.WaitPrimitive("CUTG", "[CUTG]");
      }
      {
        TPaveText text(0.5, 0.5, .85, .8, "brNDC");
        text.AddText("Please modify the cutg if needed.");
        text.AddText("Press Ctrl-C to continue");
        text.Draw(); 

        c1.Update();
        c1.Modified();
        c1.WaitPrimitive("temp", "[CUTG]");
      }

      output.cd();
      tightCut -> Write(tightCutName);

      if(BEE)
      {
        BEE -> Delete();
        BEE = nullptr;
      }
    }

    auto profile = STPIDProbTask::ProfileX(&PID, tightCut);//PID.ProfileX("_px", 1, -1, "s [CUTG]");
    //TH1F PIDMean("PIDMean", "Mean;momentum (MeV/c2);dEdX", nbinsx, momMin, momMax);
    auto PIDMean = new TGraph;
    auto PIDStd = new TGraph;

    for(int i = 1; i < PID.GetNbinsX(); ++i)
      if(profile -> GetBinContent(i) > 0) 
      {
        PIDMean -> SetPoint(PIDMean -> GetN(), profile->GetXaxis()->GetBinCenter(i), profile->GetBinContent(i));
        PIDStd -> SetPoint(PIDStd -> GetN(),  profile->GetXaxis()->GetBinCenter(i), profile->GetBinError(i));
      }
    profile -> Delete();
    profile = nullptr;

    // first check and see if there are BEE functions in the file
    c1.cd(); PID.Draw("colz");
    if(BEE) BEE -> SetRange(10, momMax);
    BEE = (TF1 *) AskRedrawIfExist(BEE, "same", &c1, TString::Format("BEE line for %s is found. Redraw? (y/n)", pname.c_str()));

    if(!BEE)
    {
      tBBE.SetNpx(5000);
      bool redraw = true;
      PIDMean -> Fit(&tBBE, "Q");
      PID.Draw("colz");
      PIDMean -> Draw("PE same");

      //while(redraw)
      TPaveText text(0.5, 0.5, .85, .8, "brNDC");
      text.AddText("Please inspect the result of the fit.");
      text.AddText("You may adjust the parameters in the fit panel.");
      text.AddText("Press Ctrl-C to continue");
      text.Draw(); 

      c1.Update();
      c1.Modified();
      c1.WaitPrimitive("temp", "[CUTG]");

      // Obtain the tf1 after adjustment is made on fit panel
      auto newBBE = (TF1*) PIDMean -> GetFunction("BBE");
      tBBE.SetParameters(newBBE->GetParameters());
      tBBE.Draw("L same");

      c1.Update();
      c1.Modified();

      output.cd(); 
      BEE = (TF1*) tBBE.Clone(BEEName);
      BEE -> SetRange(0.1, 4500);
      BEE -> Write();
      
      if(PIDSigma) PIDSigma -> Delete();
      PIDSigma = nullptr;
    }

    PIDSigma = (TF1 *) AskRedrawIfExist(PIDSigma, "", nullptr, "PID line is found. Do you want to discard it and refit error?");

    if(!PIDSigma)
    {
      std::string fitname = "ErrorFit" + pname;
      PIDSigma = new TF1(fitname.c_str(), [](double *x, double *p){return p[0]/pow(x[0], p[1]) + p[2];}, 0, 4500, 3);
      PIDSigma -> SetParameters(100,2,0);

      TCanvas c2("c2", "c2");
      PIDStd -> Draw("AP");
      PIDStd -> Fit(PIDSigma);

      TPaveText text(0.5,0.5,0.85,0.8, "brNDC");
      text.AddText("Please modify the fit with Fit Panel if needed.");
      text.AddText("Then press Ctrl-C");
      text.Draw();
      c2.WaitPrimitive("temp", "[CUTG]");

      // Obtain the tf1 after adjustment is made on fit panel
      auto newfitError = (TF1*) PIDStd -> GetFunction(fitname.c_str());
      PIDSigma -> SetParameters(newfitError->GetParameters());
      
      output.cd();
      auto clonedPIDSigma = (TF1*) PIDSigma -> Clone(PIDSigmaName);
      clonedPIDSigma -> SetRange(0.1, 4500);
      clonedPIDSigma -> Write();
    }

    // draw the acceptable range for the particle
    GLimit = (TCutG *) AskRedrawIfExist(GLimit, "L same", &c1, TString::Format("PID Limits for %s is found. Redraw? (y/n)", pname.c_str()));
 
    c1.cd(); PID.Draw("colz");
    if(!GLimit)
    {
      c1.cd();
      {
        TPaveText text(0.5,0.5,0.85,0.8, "brNDC");
        text.AddText("Please draw the acceptable range for the particle.");
        text.AddText("It can be lax because it only serves as a guild for iterative Bayes.");
        text.Draw();

        if(auto obj = c1.GetListOfPrimitives()->FindObject("CUTG")) c1.GetListOfPrimitives()->Remove(obj);
        c1.Modified();

        while(!GLimit) GLimit = (TCutG*) c1.WaitPrimitive("CUTG", "[CUTG]");
      }
      {
        TPaveText text(0.5,0.5,0.85,0.8, "brNDC");
        text.AddText("You may adjust the graphical cut.");
        text.AddText("Press Ctrl-C to continue");
        text.Draw(); 
        c1.WaitPrimitive("temp", "[CUTG]");
      }

      auto clonedGLimit = (TCutG*) GLimit -> Clone(GLimitName);
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
      //gaus.FixParameter(1, temp.GetMean());
      temp.Fit(&gaus, "Q");
    }
    else gaus.SetParameters(0,0,0);
    profile->SetBinContent(i, gaus.GetParameter(1));
    profile->SetBinError(i, gaus.GetParameter(2));
  }
  return profile;
}

void STPIDProbTask::CreatePriorFromCut(const std::string& anaFile, const std::string& priorFile)
{
  STAnaParticleDB::FillTDatabasePDG();
  struct particleCharacteristic{ int color; int pdg; TString name; };
  std::vector<particleCharacteristic> particles;
  auto db = TDatabasePDG::Instance();
  for(int i = 0; i < STAnaParticleDB::SupportedPDG.size(); ++i)
    particles.push_back({i + 1, 
                         STAnaParticleDB::SupportedPDG[i], 
                         db->GetParticle(STAnaParticleDB::SupportedPDG[i])->GetName()});

  TCanvas c1;
  // continue from where we left in the existing files
  TFile output(priorFile.c_str(), "UPDATE");
  
  c1.cd();
  int nbinsx = 1000;
  int nbinsy = 1000;
  double momMin = 0;
  double momMax = 4500;
  double minDeDx = 0;
  double maxDeDx = 5000;

  TH2F PID("PID", "PID", nbinsx, momMin, momMax, nbinsy, minDeDx, maxDeDx);
  PID.Sumw2();

  TChain chain("cbmsim");
  std::string anaFileWithSuffix = anaFile;
  if(!has_suffix(anaFile, ".root")) anaFileWithSuffix += ".root";
  // the tree name could be cbmsim or spirit
  // need to add both
  {
    chain.Add((anaFileWithSuffix + "/spirit").c_str());
    chain.Add((anaFileWithSuffix + "/cbmsim").c_str());
    // suppress all the error due to not able to read from one of those trees
    auto origVerbosity = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kFatal;
    if(chain.GetEntries() == 0) throw std::runtime_error("No entries is being read from the file!");
    gErrorIgnoreLevel = origVerbosity;
  }

  chain.SetAlias("phi", "(vaMom.Phi() > 0)? vaMom.Phi()*TMath::RadToDeg() : vaMom.Phi()*TMath::RadToDeg() + 360");
  chain.Draw("vadedx:vaMom.Mag()>>PID", 
             "vaNRowClusters + vaNLayerClusters > 15 && recodpoca.Mag() < 15 && (fabs(vaMom.Phi()*TMath::RadToDeg()) < 30 || fabs(phi - 180) < 30) ", 
             "goff",100000);

  for(const auto& part : particles)
  {
    PID.Draw("colz");
    TLatex t(2000, 800, TString::Format("Draw cut for %s", part.name.Data()));
    
    t.Draw();
    TCutG *cutg = nullptr;
    while(!cutg) cutg = (TCutG*) c1.WaitPrimitive("CUTG", "[CUTG]");
    std::cout << "Finished? Press Ctrl-C" << std::endl;
    c1.WaitPrimitive("temp", "[CUTG]"); //checkpoint to allow user to modify the cutg before pressing Ctrl-C
    
    auto cutName = part.name + "Cut";
    cutg -> SetName(cutName);
    auto proj = PID.ProjectionX("",0,-1,"[" + cutName + "]");
    output.cd();
    proj -> Write(TString::Format("Distribution%d", part.pdg)); 

    t.Delete();
  }
  
  
  std::cout << "All PID cuts are drawn." << std::endl;
}
