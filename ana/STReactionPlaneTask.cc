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
#include "TPad.h"

#include <cmath>

ClassImp(STReactionPlaneTask);

STReactionPlaneTask::STReactionPlaneTask()
{
  fLogger = FairLogger::GetLogger(); 
  fReactionPlane = new STVectorF();
  fReactionPlane -> fElements.push_back(0);
  fReactionPlaneV2 = new STVectorF();
  fReactionPlaneV2 -> fElements.push_back(0);
  fQMag = new STVectorF();
  fQMag -> fElements.push_back(0);

  fPhiEff = new TClonesArray("STVectorF");
  fV1RPAngle = new TClonesArray("STVectorF");
  fV2RPAngle = new TClonesArray("STVectorF");
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    new((*fPhiEff)[i]) STVectorF();
    new((*fV1RPAngle)[i]) STVectorF();
    new((*fV2RPAngle)[i]) STVectorF();
  }
}

STReactionPlaneTask::~STReactionPlaneTask()
{}

double STReactionPlaneTask::Correction(const TVector2& Q_vec)
{
  double phi = -9999;
  if(Q_vec.Mod() > 0)
  {
    if(fShift && fQx_mean && fQy_mean) 
    {
      phi = this -> Shifting(Q_vec);
      if(fFlat && fAn && fBn) phi = this -> Flattening(phi);
    }
    else
      phi = Q_vec.Phi();
    phi = (phi > TMath::Pi())? phi - 2*TMath::Pi() : phi;
  }
  else phi = -9999;
  return phi;
}

double STReactionPlaneTask::Shifting(TVector2 Q_vec)
{
  Q_vec.SetX((Q_vec.X() - fQx_mean -> GetVal())/fQx_sigma -> GetVal());
  Q_vec.SetY((Q_vec.Y() - fQy_mean -> GetVal())/fQy_sigma -> GetVal());
  return  Q_vec.Phi();
}

double STReactionPlaneTask::Flattening(double phi)
{
  double delta_phi = 0;
  for(int i = 0; i < fAn -> fElements.size(); ++i)
  {
    int n = i + 1;
    delta_phi += fAn -> fElements[i]*cos(n*phi) + fBn -> fElements[i]*sin(n*phi);
  }
  return phi + delta_phi;
}

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
      else fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Efficiency histogram of particle %d is loaded", pdg));
    }
    if(auto temp = (TParameter<int>*) fEffFile -> Get("NClus")) fMinNClusters = temp -> GetVal();
    if(auto temp = (TParameter<double>*) fEffFile -> Get("DPOCA")) fMaxDPOCA = temp -> GetVal();
  }
  if(!fBiasFilename.empty())
  {
    fLogger -> Info(MESSAGE_ORIGIN, ("Bias correction file is loaded from " + fBiasFilename).c_str());
    fBiasFile = new TFile(fBiasFilename.c_str());
    fQx_mean = (TParameter<double>*) fBiasFile -> Get("Qx_mean"); 
    fQy_mean = (TParameter<double>*) fBiasFile -> Get("Qy_mean"); 
    fQx_sigma = (TParameter<double>*) fBiasFile -> Get("Qx_sigma");
    fQy_sigma = (TParameter<double>*) fBiasFile -> Get("Qy_sigma");
    fAn = (STVectorF*) fBiasFile -> Get("An");
    fBn = (STVectorF*) fBiasFile -> Get("Bn");
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
  ioMan -> Register("RPV2", "ST", fReactionPlaneV2, fIsPersistence);
  ioMan -> Register("QMag", "ST", fQMag, fIsPersistence);
  ioMan -> Register("PhiEff", "ST", fPhiEff, fIsPersistence);
  ioMan -> Register("V1RPAngle", "ST", fV1RPAngle, fIsPersistence);
  ioMan -> Register("V2RPAngle", "ST", fV2RPAngle, fIsPersistence);

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
  std::vector<std::vector<TVector2>> Q_v1_elements(fSupportedPDG.size()), Q_v2_elements(fSupportedPDG.size());
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
      TVector2 Q_v1(0, 0), Q_v2(0, 0);
      double weight = 0;
      double eff = 1;
      const auto& vec = cmVector -> fElements[j];
      double theta = vec.Theta();
      double phi = vec.Phi();
      if(TEff && !std::isnan(theta)) eff = TEff -> Interpolate(theta, phi);
      if(!(data -> vaNRowClusters[j] + data -> vaNLayerClusters[j] > fMinNClusters && data -> recodpoca[j].Mag() < fMaxDPOCA)) eff = 0;
      phiEff.push_back(eff);

      if(fabs(2*rap -> fElements[j]/beamRap) > fMidRapidity) // reject mid-rapidity particles
        if(vec.Theta() < fThetaCut)
        {
          double prob_ele = prob -> fElements[j];
          weight = (std::abs(fSupportedPDG[i]) == 211 || eff == 0)? 0 : prob_ele/eff;

          // coefficients of the generalized Q-vector method
          if(fParticleCoef.size() == fSupportedPDG.size()) weight *= fParticleCoef[i];
          else weight *= fChargeCoef*charge + fMassCoef*mass + fConstCoef;

          // front size pos, back side neg
          double mag = vec.Perp();
          Q_v1.SetMagPhi(mag, phi);
          Q_v2.SetMagPhi(mag, 2*phi);
          if(vec.z() < 0) weight = -weight;
        }
      Q_v1_elements[i].push_back(weight*Q_v1);
      Q_v2_elements[i].push_back(weight*Q_v2);
    }
  }
  TVector2 Q_v1_sum(0, 0), Q_v2_sum(0, 0);
  for(const auto& Q_v1 : Q_v1_elements) 
    for(const auto& Q : Q_v1) Q_v1_sum += Q;
  for(const auto& Q_v2 : Q_v2_elements) 
    for(const auto& Q : Q_v2) Q_v2_sum += Q;

  {
    fQMag -> fElements[0] = Q_v1_sum.Mod();
    fReactionPlane -> fElements[0] = this -> Correction(Q_v1_sum);
    auto phi = (Q_v2_sum.Mod() > 0)? Q_v2_sum.Phi()/2 : -9999;
    fReactionPlaneV2 -> fElements[0] = (phi > TMath::Pi())? phi - 2*TMath::Pi() : phi;//0.5*((phi > TMath::Pi())? phi - 2*TMath::Pi() : phi);
  }

  double mcRotZ = (fUseMCReactionPlane)? gRandom -> Gaus(fMCRotZ -> fElements[0], fMCReactionPlaneRes) : 0;
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
    int mult = cmVector -> fElements.size();
    auto v1RPAngle = static_cast<STVectorF*>(fV1RPAngle -> At(i));
    v1RPAngle -> fElements.clear();
    auto v2RPAngle = static_cast<STVectorF*>(fV2RPAngle -> At(i));
    v2RPAngle -> fElements.clear();

    for(int j = 0; j < mult; ++j)
    {
      // eliminate correlation by removing the particle of interest constribution to reaction plane determination
      double phi = (fUseMCReactionPlane)? mcRotZ : this -> Correction((Q_v1_sum - Q_v1_elements[i][j]));
      v1RPAngle -> fElements.push_back((phi > TMath::Pi())? phi - 2*TMath::Pi() : phi);
      phi = (fUseMCReactionPlane)? mcRotZ : (Q_v2_sum - Q_v2_elements[i][j]).Phi()/2;
      v2RPAngle -> fElements.push_back((phi > TMath::Pi())? phi - 2*TMath::Pi() : phi);
    }
  }
}

void STReactionPlaneTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  outfile -> cd();
  if(fQx_mean && fQx_sigma && fQy_mean && fQy_sigma)
  {
    fQx_mean -> Write();
    fQy_mean -> Write();
    fQx_sigma -> Write();
    fQy_sigma -> Write();
  }
  if(fAn && fBn)
  {
    fAn -> Write("An");
    fBn -> Write("Bn");
  }
}

void STReactionPlaneTask::LoadPhiEff(const std::string& eff_filename)
{ fEffFilename = eff_filename; }

void STReactionPlaneTask::LoadBiasCorrection(const std::string& bias_filename)
{ fBiasFilename = bias_filename; }

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
      if(file.Get("Qx_mean"))
      {
        std::cerr << "The supplied file has been flattened. PhiEfficiency must be applied on files BEFORE flattening. ABORT.\n";
        return;
      }
    }
  }
      
  TFile output(out_filename.c_str(), "RECREATE");
  TParameter<int> NClus("NClus", nClus);
  TParameter<double> DPOCA("DPOCA", poca);
  NClus.Write();
  DPOCA.Write();
  for(int i = 0; i < STAnaParticleDB::GetSupportedPDG().size(); ++i)
  {
    auto hist_name = TString::Format("PhiEfficiency%d", STAnaParticleDB::GetSupportedPDG()[i]);
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
    output.cd();
    hist.Write();
  }
}

void STReactionPlaneTask::CreateBiasCorrection(const std::string& ana_filename, const std::string& out_filename)
{
  TChain chain("cbmsim");
  chain.Add(ana_filename.c_str());

  // check if the momentum vector has been rotated
  // can only create efficiency if the data is NOT rotated
  TParameter<double> *Qx_mean = nullptr;
  TParameter<double> *Qy_mean = nullptr;
  TParameter<double> *Qx_sigma = nullptr;
  TParameter<double> *Qy_sigma = nullptr;
  STVectorF *An = nullptr;
  STVectorF *Bn = nullptr;
  {
    auto fileList = chain.GetListOfFiles();
    for(int i = 0; i < fileList->GetEntries(); ++i)
    {
      auto filename = fileList -> At(i) -> GetTitle();
      TFile file(filename);
      auto taskList = static_cast<FairFileHeader*>(file.Get("FileHeader")) -> GetListOfTasks();
      if(!taskList -> FindObject("STReactionPlaneTask"))
      {
        std::cerr << "STReactionPlaneTask is NOT found in " << filename << std::endl; 
        std::cerr << "This means the file doesn't contain raw reaction plane to which corrections will be applied to.\n";
        std::cerr << "ABORT. Please try again for files that is processed by STReactionPlaneTask.\n";
        return;
      }
      Qx_mean = (TParameter<double>*) file.Get("Qx_mean");
      Qy_mean = (TParameter<double>*) file.Get("Qy_mean");
      Qx_sigma = (TParameter<double>*) file.Get("Qx_sigma");
      Qy_sigma = (TParameter<double>*) file.Get("Qy_sigma");

      An = (STVectorF*) file.Get("An");
      Bn = (STVectorF*) file.Get("Bn");
    }
    if(!Qx_mean || !Qy_mean || !Qx_sigma || !Qy_sigma)
      std::cerr << "No Q-vector mean and std is found in the file. They will be generated.\n";
    else
      std::cerr << "Q-vector distribution infomation is loaded.\n";
    if(An || Bn)
      std::cerr << "The file has been Fourier transformed. It will not be done again.\n";
    else
      std::cerr << "The distribution will be flatted with Fourier series.\n";
    
  }
 
  // apply mizuki-san's two corrections
  // correction 1: shifting, need mean and std of the distribution
  if(!Qx_mean || !Qy_mean || !Qx_sigma || !Qy_sigma)
  {
    TH2F hist("Qxy_dist", "", 1000, -10000, 10000, 1000, -10000, 10000);
    chain.Project("Qxy_dist", "QMag.fElements*sin(RP.fElements):QMag.fElements*cos(RP.fElements)");
    Qx_mean = new TParameter<double>("Qx_mean", hist.GetMean(1));
    Qx_sigma = new TParameter<double>("Qx_sigma", hist.GetRMS(1));
    Qy_mean = new TParameter<double>("Qy_mean", hist.GetMean(2));
    Qy_sigma = new TParameter<double>("Qy_sigma", hist.GetRMS(2));
  }
  TFile output(out_filename.c_str(), "RECREATE");
  output.cd();
  Qx_mean -> Write();
  Qx_sigma -> Write();
  Qy_mean -> Write();
  Qy_sigma -> Write();

  // correction 2: flattening with Fourier expansion
  if(!An || !Bn)
  {
    auto Qx_shifted = TString::Format("(QMag.fElements*cos(RP.fElements) - %f)/%f", Qx_mean -> GetVal(), Qx_sigma -> GetVal());
    auto Qy_shifted = TString::Format("(QMag.fElements*sin(RP.fElements) - %f)/%f", Qy_mean -> GetVal(), Qy_sigma -> GetVal());
    auto Phi_shifted = "TMath::ATan2(" + Qy_shifted + ", " + Qx_shifted + ")";

    //calculate fourier series
    An = new STVectorF();
    Bn = new STVectorF();
    for(int i = 1; i <= 5; ++i)
    {
      TH2F hist("fourier", "", 1000, -1.2, 1.2, 1000, -1.2, 1.2);
      chain.Project("fourier", TString::Format("sin(%d*%s):cos(%d*%s)", i, Phi_shifted.Data(), i, Phi_shifted.Data()));
      An -> fElements.push_back(-hist.GetMean(2)*2./i);
      Bn -> fElements.push_back(hist.GetMean(1)*2./i);
    }
  }
  output.cd();
  An -> Write("An");
  Bn -> Write("Bn");
  
}

double STReactionPlaneTask::ReactionPlaneRes(const std::string& filename1, const std::string& filename2)
{
  // if filename2 is empty, the script will assume that the data comes from MC and reaction plane will be loaded directly from the truth value
  TH1F *hist = new TH1F("hist", "hist", 180, 0, 180);

  TFile file1(filename1.c_str());
  TFile file2(filename2.c_str());

  auto tree1 = (TTree*) file1.Get("cbmsim");
  auto tree2 = (TTree*) file2.Get("cbmsim");

  int n = tree1 -> Draw((tree2)? "RP.fElements:RPV2.fElements" : "RP.fElements:RPV2.fElements:MCRotZ.fElements", "", "goff");
  if(tree2) tree2 -> Draw("RP.fElements:RPV2.fElements", "", "goff");

  auto v1_file1 = tree1 -> GetV1();
  auto v1_file2 = (tree2)? tree2 -> GetV1() : tree1 -> GetV3();
  auto v2_file1 = tree1 -> GetV2();
  auto v2_file2 = (tree2)? tree2 -> GetV1() : tree1 -> GetV3();

  double ave_cos_v1 = 0;
  double ave_cos2_v1 = 0;
  double ave_cos2_v2 = 0;
  for(int i = 0; i < n; ++i)
  {
    if(!std::isnan(v1_file1[i]) && !std::isnan(v1_file2[i]))
    {
      ave_cos_v1 += cos(v1_file1[i] - v1_file2[i]);
      ave_cos2_v1 += cos(2*(v1_file1[i] - v1_file2[i]));
      ave_cos2_v2 += cos(2*(v2_file1[i] - v2_file2[i]));
    }
  }

  std::cout << "<cos(phi1_1 - phi2_1)> = " << ave_cos_v1/n << std::endl;
  std::cout << "<cos(2(phi1_1 - phi2_1))> = " << ave_cos2_v1/n << std::endl;
  std::cout << "<cos(2(phi1_2 - phi2_2))> = " << ave_cos2_v2/n << std::endl;

  return 0;
}

