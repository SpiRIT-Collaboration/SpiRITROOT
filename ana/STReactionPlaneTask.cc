#include "STReactionPlaneTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairFileHeader.h"

#include "TRandom.h"
#include "STVector.hh"
#include "TDatabasePDG.h"
#include "TParameter.h"
#include "TF1.h"
#include "TH2.h"

#include <cmath>

ClassImp(STReactionPlaneTask);

STReactionPlaneTask::STReactionPlaneTask()
{
  fLogger = FairLogger::GetLogger(); 
  fReactionPlane = new STVectorF();
  fReactionPlane -> fElements.push_back(0);

  fPhiEff = new TClonesArray("STVectorF");
  for(int i = 0; i < fSupportedPDG.size(); ++i) new((*fPhiEff)[i]) STVectorF();
}

STReactionPlaneTask::~STReactionPlaneTask()
{}

InitStatus STReactionPlaneTask::Init()
{
  STAnaParticleDB::FillTDatabasePDG();
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  if(!fEffFilename.empty())
  {
    fLogger -> Info(MESSAGE_ORIGIN, ("Phi Efficiency file is loaded from " + fEffFilename).c_str());
    fEffFile = new TFile(fEffFilename.c_str());
    for(auto pdg : fSupportedPDG)
    {
      fEff[pdg] = (TH2F*) fEffFile -> Get(TString::Format("PhiEfficiency%d", pdg));
      if(!fEff[pdg]) fLogger -> Fatal(MESSAGE_ORIGIN, "Efficiency histogram cannot be loaded. Will ignore efficiency");
    }
    if(auto temp = (TParameter<int>*) fEffFile -> Get("NClus")) fMinNClusters = temp -> GetVal();
    if(auto temp = (TParameter<double>*) fEffFile -> Get("DPOCA")) fMaxDPOCA = temp -> GetVal();
  }
  fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Theta cut for reaction plane < %f deg", fThetaCut));
  fThetaCut = fThetaCut*TMath::DegToRad();

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fFragRapidity = (TClonesArray*) ioMan -> GetObject("FragRapidity");
  fBeamRapidity = (STVectorF*) ioMan -> GetObject("BeamRapidity");
  fMCRotZ = (STVectorF*) ioMan -> GetObject("MCRotZ");

  if(fUseMCReactionPlane)
  {
    if(fMCRotZ) fLogger -> Info(MESSAGE_ORIGIN, "Rotate particles according to the MC truth reaction plane.");
    else
    {
      fLogger -> Info(MESSAGE_ORIGIN, "UseMCReactionPlane is set but cannot load MC truth reaction plane. Will use inferred reaction plane instead");
      fUseMCReactionPlane = false;
    }
  }

  ioMan -> Register("RP", "ST", fReactionPlane, fIsPersistence);
  ioMan -> Register("PhiEff", "ST", fPhiEff, fIsPersistence);

  return kSUCCESS;
}

void STReactionPlaneTask::SetParContainers()
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

void STReactionPlaneTask::Exec(Option_t *opt)
{
  // use q vector method. Assume no efficiency problem
  TVector2 Q(0,0);
 
  std::vector<std::vector<TVector2>> Q_elements(fSupportedPDG.size());
  auto beamRap = fBeamRapidity -> fElements[1];
  auto data = static_cast<STData*>(fData -> At(0));
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto& phiEff = static_cast<STVectorF*>(fPhiEff -> At(i)) -> fElements;
    phiEff.clear();
    auto cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
    auto prob = static_cast<STVectorF*>(fProb -> At(i));
    auto rap = static_cast<STVectorF*>(fFragRapidity -> At(i));
    int mult = cmVector -> fElements.size();
    auto pinfo = TDatabasePDG::Instance() -> GetParticle(fSupportedPDG[i]);
    auto mass = pinfo -> Mass()/STAnaParticleDB::kAu2Gev;
    auto charge = pinfo -> Charge()/3.;
    int pdg = fSupportedPDG[i];
    auto TEff = fEff[pdg];

    for(int j = 0; j < mult; ++j)
    {
      double eff = 1;
      const auto& vec = cmVector -> fElements[j];
      double theta = vec.Theta();
      double phi = vec.Phi();
      if(TEff && !std::isnan(theta)) eff = TEff -> Interpolate(vec.Theta(), vec.Phi());
      if(!(data -> vaNRowClusters[j] + data -> vaNLayerClusters[j] > fMinNClusters && data -> recodpoca[j].Mag() < fMaxDPOCA)) eff = 0;
      phiEff.push_back(eff);

      if(fabs(2*rap -> fElements[j]/beamRap) > fMidRapidity) // reject mid-rapidity particles
      {
        if(vec.Theta() < fThetaCut)
        {
          double prob_ele = prob -> fElements[j];
          double weight = (std::abs(fSupportedPDG[i]) == 211 || eff == 0)? 0 : prob_ele/eff;

          // coefficients of the generalized Q-vector method
          if(fParticleCoef.size() == fSupportedPDG.size()) weight *= fParticleCoef[i];
          else weight *= fChargeCoef*charge + fMassCoef*mass + fConstCoef;

          // front size pos, back side neg
          if(vec.z() > 0) Q_elements[i].push_back(weight*vec.XYvector());
          else Q_elements[i].push_back(-weight*vec.XYvector());
          Q += Q_elements[i].back();
        }
        else Q_elements[i].emplace_back(0, 0);
      }
      else Q_elements[i].emplace_back(0, 0);
    }
  }
  if(sqrt(Q.X()*Q.X() + Q.Y()*Q.Y()) > 0) 
  {
    auto phi = Q.Phi();
    fReactionPlane -> fElements[0] = (phi > TMath::Pi())? phi - 2*TMath::Pi() : phi;
  }
  else fReactionPlane -> fElements[0] = -9999;

  double mcRotZ = (fUseMCReactionPlane)? gRandom -> Gaus(fMCRotZ -> fElements[0], fMCReactionPlaneRes) : 0;
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
    int mult = cmVector -> fElements.size();
    for(int j = 0; j < mult; ++j)
    {
      // eliminate correlation by removing the particle of interest constribution to reaction plane determination
      double phi = (fUseMCReactionPlane)? mcRotZ : (Q - Q_elements[i][j]).Phi();
      cmVector -> fElements[j].RotateZ(-phi);
    }
  }
}

void STReactionPlaneTask::LoadPhiEff(const std::string& eff_filename)
{ fEffFilename = eff_filename; }

void STReactionPlaneTask::CreatePhiEffFromData(const std::string& ana_filename, const std::string& out_filename, int nClus, double poca)
{
  TChain chain("cbmsim");
  chain.Add(ana_filename.c_str());

  // check if the momentum vector has been rotated
  // can only create efficiency if the data is NOT rotated
  {
    auto fileList = chain.GetListOfFiles();
    for(int i = 0; i < fileList->GetEntries(); ++i)
    {
      auto filename = fileList -> At(i) -> GetTitle();
      TFile file(filename);
      auto taskList = static_cast<FairFileHeader*>(file.Get("FileHeader")) -> GetListOfTasks();
      if(taskList -> FindObject("STReactionPlaneTask"))
      {
        std::cerr << "STReactionPlaneTask is found in " << filename << std::endl; 
        std::cerr << "This means the even has been rotated. Phi efficiency can only be extracted from files WITHOUT rotation.\n";
        std::cerr << "ABORT. Please try again for files that is not processed by STReactionPlaneTask.\n";
        return;
      }
    }
  }
      
  TFile output(out_filename.c_str(), "RECREATE");
  TParameter<int> NClus("NClus", nClus);
  TParameter<double> DPOCA("DPOCA", poca);
  NClus.Write();
  DPOCA.Write();
  for(int i = 0; i < STAnaParticleDB::SupportedPDG.size(); ++i)
  {
    auto hist_name = TString::Format("PhiEfficiency%d", STAnaParticleDB::SupportedPDG[i]);
    TH2F hist(hist_name, "", 40, 0, 3.15, 100, -3.15, 3.15);
    chain.Project(hist_name, TString::Format("CMVector[%d].fElements.Phi():CMVector[%d].fElements.Theta()", i, i), TString::Format("Prob[%d].fElements*(Prob[%d].fElements > 0.2 && STData[0].vaNRowClusters + STData[0].vaNLayerClusters >%d &&STData[0].recodpoca.Mag() < %f)", i, i, nClus, poca));
    
    // normalize
    hist.Smooth();
    for(int j = 0; j <= hist.GetNbinsX(); ++j)
    {
      // normalize each x-bin
      auto proj = hist.ProjectionY("_px", j, j);
      double max = proj -> GetMaximum();
      max = (max == 0)? 1 : max;
      proj -> Delete();
      for(int k = 0; k <= hist.GetNbinsY(); ++k) 
        hist.SetBinContent(j, k, hist.GetBinContent(j, k)/max);
    }
    hist.Write();
  }
}

double STReactionPlaneTask::ReactionPlaneRes(const std::string& filename1, const std::string& filename2)
{
  // if filename2 is empty, the script will assume that the data comes from MC and reaction plane will be loaded directly from the truth value
  TH1F *hist = new TH1F("hist", "hist", 180, 0, 180);

  TFile file1(filename1.c_str());
  TFile file2(filename2.c_str());

  auto tree1 = (TTree*) file1.Get("cbmsim");
  auto tree2 = (TTree*) file2.Get("cbmsim");

  int n = tree1 -> Draw((tree2)? "RP.fElements*TMath::RadToDeg()" : "RP.fElements*TMath::RadToDeg():MCRotZ.fElements*TMath::RadToDeg()", "", "goff");
  if(tree2) tree2 -> Draw("RP.fElements*TMath::RadToDeg()", "", "goff");

  auto x1 = tree1 -> GetV1();
  auto x2 = (tree2)? tree2 -> GetV1() : tree1 -> GetV2();

  for(int i = 0; i < n; ++i)
    hist -> Fill(fabs(x1[i] -  x2[i]));

  hist -> Draw("hist");

  TF1 *tf1 = new TF1("tf1", "[0]*exp(-0.5*pow(x/[1],2)) + [2]*exp(-0.5*pow((x-180)/[1],2))", 0, 180);
  tf1->SetParameters(hist -> GetMaximum(),10,10);
  hist->Fit(tf1, "Q");
  tf1->SetRange(0, 180);
  tf1->Draw("l same");

  return tf1 -> GetParameter(1)*((tree2)? 0.5 : 1); // if sub events are provided, the fitted sigma is twice the detector resolution
}

