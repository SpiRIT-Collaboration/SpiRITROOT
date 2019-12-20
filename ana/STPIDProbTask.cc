#include "TROOT.h"
#include "TLatex.h"
#include "STPIDProbTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "STVector.hh"

ClassImp(STPIDProbTask);

STPIDProbTask::STPIDProbTask()
{ 
  fLogger = FairLogger::GetLogger(); 
  fPDGLists = new STVectorI();
  fMassLists = new STVectorF();

  //fBEETemp = [](double *x, double *par)
  //{
  //  double fitval;
  //  double beta = x[0]/sqrt(x[0]*x[0] + 931.5*par[0]);
  //  return par[1]/pow(beta, par[4])*(par[2] - pow(beta, par[4]) - log(par[3] + pow(931.5*par[0]/x[0], par[5])));
  //};

  for(int pdg : fSupportedPDG)
  {
    //TF1 bbe(TString::Format("Particle%d", pdg), fBEETemp, 0, 3000, 6);
    //TF1 sigma(TString::Format("ParticleSigma%d", pdg), "[0]/pow(x,[1]) + [2]");
    //switch(pdg)
    //{
    //  case 2212: 
    //    bbe.SetParameters(1, -718.7, 11.963, 59024.7, -48.037, 5.8289); 
    //    bbe.FixParameter(0, 1); 
    //    sigma.SetParameters(6.94034e5, 1.81139, 1.45987);
    //    break;
    //  case 1000010020: 
    //    bbe.SetParameters(2, -808.97, 13.02, 169219, -83.9262, 6.1987); 
    //    bbe.FixParameter(0, 2); 
    //    sigma.SetParameters(2.83664e7, 2.1745, 2.29088);
    //    break;
    //  case 1000010030: 
    //    bbe.SetParameters(3, -1233.4, 13.3135, 226629, -79.5201, 5.3072); 
    //    bbe.FixParameter(0, 3); 
    //    sigma.SetParameters(6.0944e7, 2.1798, 3.09216);
    //    break;
    //  case 1000020030: 
    //    bbe.SetParameters(3, -739.772, 14.6252, 944669, -188.571,7.8113); 
    //    bbe.FixParameter(0, 3); 
    //    sigma.SetParameters(1.70893e6, 1.62724, 2.7622);
    //    break;
    //  case 1000020040:
    //    bbe.SetParameters(4, -748.151, 14.5728, 908830, -247.735, 7.91);
    //    bbe.FixParameter(0, 4);
    //    sigma = TF1(TString::Format("ParticleSigma%d", pdg), "[0]*x*x + [1]*x + [2]");
    //    sigma.SetParameters(3.93471e-5, -1.26e-1, 1.2049e2);
    //    break;
    //  default: break;
    //}
    fFlattenHist[pdg] = TH2F(TString::Format("Part%d", pdg), "PID;momentum;dedx", 40, 0, 3000,100, -100, 100);
    //fSigma[pdg] = sigma;
    //
    //fBBE[pdg] = bbe;
  }
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
  {
    auto prob = new STVectorF();
    fPDGProb[pdg] = prob;
    ioMan -> Register(TString::Format("Prob%d", pdg), "ST", prob, fIsPersistence);
    fMomPosteriorDistribution[pdg] = TH1F(TString::Format("PartPostMom%d", pdg), ";Momentum;", 40, 0, 3000);
  }

  ioMan -> Register("PDG", "ST", fPDGLists, fIsPersistence);
  ioMan -> Register("Mass", "ST", fMassLists, fIsPersistence);
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
  fPDGLists -> fElements.clear();
  fMassLists -> fElements.clear();

  for(auto& prob : fPDGProb) prob.second->fElements.clear();

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

    std::map<int, double> PDGProb;
    double sumProb = 0;
    double yield = 1;
    for(int pdg : fSupportedPDG)
    {
      if(fMomPriorDistribution.size() == fSupportedPDG.size())
      {
        auto hist = fMomPriorDistribution[pdg];
        if(hist) yield = hist -> GetBinContent(hist -> GetXaxis() -> FindBin(momMag));
      }
      double prob = yield*TMath::Gaus(dedx, fBBE[pdg]->Eval(momMag), fSigma[pdg]->Eval(momMag));
      if(std::isnan(prob)) prob = 0;

      if(fabs(momMag - 1725) < 100 && (pdg == 1000010020 || pdg == 1000010030)) std::cout << prob << " yield " << yield << " mom " << momMag << " dedx " << dedx << " BBE " << fBBE[pdg]->Eval(momMag) << " Sigma " << fSigma[pdg]->Eval(momMag) << std::endl;

      sumProb += prob;
      PDGProb[pdg] = prob;
    }
    if(sumProb == 0) sumProb = 1;

    for(int pdg : fSupportedPDG)
    { 
      fPDGProb[pdg] -> fElements.push_back(PDGProb[pdg]/sumProb);
    }
 
    int optimumPDG = 0;
    int optimumMass = 0;
    for(int pdg : fSupportedPDG)
    {
      double prob = PDGProb[pdg]/sumProb;
      if(prob > 0.5)
      {
        optimumPDG = pdg;
        switch(pdg)
        {
          case 2212: optimumMass = 1; break;
          case 1000010020: optimumMass = 2; break;
          case 1000010030: optimumMass = 3; break;
          case 1000020030: optimumMass = 3; break;
          case 1000020040: optimumMass = 4; break;
          default: break;
        }
        if(nClus > fMinNClus && poca < fMaxDPOCA)
        {
          if(prob > 0.99)
            fMomPosteriorDistribution[pdg].Fill(momMag);
          for(auto& hist : fFlattenHist)
          {
            double expDedx = fBBE[hist.first]->Eval(momMag);
            hist.second.Fill(momMag, dedx - expDedx);
          }
        }
        break;
      }
    }
    fPDGLists -> fElements.push_back(optimumPDG);
    fMassLists -> fElements.push_back(optimumMass);
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
    std::cout << fBBE[pdg] << std::endl;
    fBBE[pdg]->FixParameter(0, fBBE[pdg]->GetParameter(0));
    fSigma[pdg] = static_cast<TF1*>(fFitFile -> Get(TString::Format("PIDSigma%d", pdg)));
  }
}
