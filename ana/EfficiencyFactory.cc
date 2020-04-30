#include "STData.hh"
#include "TH2.h"
#include "TChain.h"
#include "EfficiencyFactory.hh"
#include "TDatabasePDG.h"
#include "STAnaParticleDB.hh"
#include "TLorentzVector.h"
#include "TPad.h"
#include <glob.h>

std::vector<std::string> glob(const char *pattern) {
    glob_t g;
    glob(pattern, GLOB_TILDE, nullptr, &g); // one should ensure glob returns 0!
    std::vector<std::string> filelist;
    filelist.reserve(g.gl_pathc);
    for (size_t i = 0; i < g.gl_pathc; ++i) {
        filelist.emplace_back(g.gl_pathv[i]);
    }
    globfree(&g);
    return filelist;
}

   
EfficiencyFactory& EfficiencyFactory::SetMomBins(double t_min, double t_max, int t_bins) 
{ mom_bin_ = {t_min, t_max, t_bins}; return *this;};
EfficiencyFactory& EfficiencyFactory::SetThetaBins(double t_min, double t_max, int t_bins) 
{ theta_bin_ = {t_min, t_max, t_bins}; return *this;};

EfficiencyFactory& EfficiencyFactory::SetPtBins(double t_min, double t_max, int t_bins) 
{ pt_bin_ = {t_min, t_max, t_bins}; return *this;};
EfficiencyFactory& EfficiencyFactory::SetCMzBins(double t_min, double t_max, int t_bins) 
{ CMz_bin_ = {t_min, t_max, t_bins}; return *this;};

EfficiencyFactory& EfficiencyFactory::SetPhiCut(const std::vector<std::pair<double, double>>& t_phi_cut)
{ phi_cut_ = t_phi_cut; return *this;}
EfficiencyFactory& EfficiencyFactory::SetTrackQuality(int t_nclus, double t_poca)
{ dist_2_vert_ = t_poca; num_clusters_ = t_nclus; return *this;}

TH2F* EfficiencyFactory::UpScaling(TH2F* t_hist, double t_xfactor, double t_yfactor)
{
  auto xaxis = t_hist -> GetXaxis();
  auto yaxis = t_hist -> GetYaxis();
  auto scaledHist = new TH2F(TString::Format("%s_Scaled", t_hist->GetName()), t_hist->GetName(), 
                             t_xfactor*xaxis -> GetNbins(), xaxis -> GetXmin(), xaxis -> GetXmax(),
                             t_yfactor*yaxis -> GetNbins(), yaxis -> GetXmin(), yaxis -> GetXmax());
  for(int i = 1; i < scaledHist -> GetNbinsX(); ++i)
    for(int j = 1; j < scaledHist -> GetNbinsY(); ++j)
    {
      double x = scaledHist -> GetXaxis() -> GetBinCenter(i);
      double y = scaledHist -> GetYaxis() -> GetBinCenter(j);
      scaledHist -> SetBinContent(i, j, t_hist -> Interpolate(x, y));
    }
  return scaledHist;
}

/*********************************************
*  OrigEfficiency
**********************************************/
OrigEfficiencyFactory::OrigEfficiencyFactory(const std::string& t_efficiency_db) : data_filename_(t_efficiency_db)
{}
TEfficiency OrigEfficiencyFactory::FinalizeBins(int t_pdg,
                                            bool t_verbose)
{
  TFile file(data_filename_.c_str());
  if(!file.IsOpen()) throw std::runtime_error("EfficiencyFactory: File " + data_filename_ + " not found.");

  std::string treename;
  switch(t_pdg)
  {  
    case 211: treename = "pi+"; break;
    case -211: treename = "pi-"; break;
    case 2212: treename = "proton"; break;
    case 1000010020: treename = "deuteron"; break;
    case 1000010030: treename = "triton"; break;
    case 1000020030: treename = "he3"; break;
    default:
      throw std::runtime_error("PDG " + std::to_string(t_pdg) + " is not yet supported");
  }

  auto tree = (TTree*) file.Get(treename.c_str());
  if(!tree) throw std::runtime_error("EfficiencyFactory: TTree \"" + treename + "\" not found in " + data_filename_);

  TTreeReader reader(tree);
  typedef TTreeReaderValue<double> dvalue;
  typedef TTreeReaderValue<int> ivalue;
  typedef TTreeReaderValue<bool> bvalue;

  std::string prefix = "VA";

  dvalue P(reader, "P"), Theta(reader, "Theta"), Phi(reader, "Phi");
  dvalue P_reco(reader, (prefix + "P_reco").c_str()); 
  dvalue Theta_reco(reader, (prefix + "Theta_reco").c_str()); 
  dvalue Phi_reco(reader, (prefix + "Phi_reco").c_str());
  dvalue dist_to_vertex(reader, "DistToVertex"), dEdX(reader, "DeDx");
  ivalue clusters(reader, "ClusterNum"), charge(reader, "Charge"), mult(reader, "mult");
  bvalue detected(reader, "Detected");

  TEfficiency efficiency("tefficiency", 
                         "tefficiency",
                         mom_bin_.nbins, 
                         mom_bin_.min, 
                         mom_bin_.max,
                         theta_bin_.nbins, 
                         theta_bin_.min, 
                         theta_bin_.max);

  while(reader.Next())
  {
    if(t_verbose) 
    {
      std::cout << "EfficiencyFactory progress: " << std::setw(3) << std::setfill(' ') << int(100*(reader.GetCurrentEntry()+1)/(double)reader.GetEntries(true)) << " %  \r";
      if(reader.GetEntries(true) == reader.GetCurrentEntry()+1) std::cout << std::flush;
    }
      

    // check cut conditions
    bool satisfy_cuts = false;
    bool insidePhi = false;
    for(const auto& phiCut : phi_cut_) 
      if(phiCut.first < *Phi_reco && *Phi_reco < phiCut.second)
      {
        insidePhi = true;
        break;
      }
    if(insidePhi && *clusters > num_clusters_
       && *dist_to_vertex < dist_2_vert_
       && *detected)  
    {
      satisfy_cuts = true;
    }else satisfy_cuts = false;
    efficiency.Fill(satisfy_cuts, *P, *Theta);
  }
  efficiency.SetDirectory(0);
  return efficiency;
}

/********************************************************
* Efficiency from ana 
*********************************************************/

EfficiencyFromAnaFactory::EfficiencyFromAnaFactory(){}

void EfficiencyFromAnaFactory::SetDataBaseForPDG(int t_pdg, const std::string& t_efficiency_db)
{
  fEfficiencyDB[t_pdg] = t_efficiency_db;
}

TEfficiency EfficiencyFromAnaFactory::FinalizeBins(int t_pdg,
                                                   bool t_verbose)
{
  TChain tree("cbmsim");
  for(const auto& filename : glob(fEfficiencyDB[t_pdg].c_str()))
    tree.Add(filename.c_str());

  TH2F *det = new TH2F("det", "det", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  TH2F *truth = new TH2F("truth", "truth", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  
  TString condition = TString::Format("STData[0].vaEmbedTag && PDG.fElements == %d && ", t_pdg);
  TString wrappedPhi = "((STData[0].vaMom.Phi() < 0)? STData[0].vaMom.Phi()*TMath::RadToDeg() + 360:STData[0].vaMom.Phi()*TMath::RadToDeg())";
  condition += "(";
  for(int i = 0; i < phi_cut_.size(); ++i)
  {  
    if(i > 0) condition += ") || ";
    condition += TString::Format("(%s > %f && %f > %s",
                                 wrappedPhi.Data(), phi_cut_[i].first, phi_cut_[i].second, wrappedPhi.Data());
  }
  condition += "))";

  condition += TString::Format(" && STData[0].vaNRowClusters + STData[0].vaNLayerClusters > %d && STData[0].recodpoca.Mag() < %f",
                               num_clusters_, dist_2_vert_);

  tree.Project("det", "STData[0].embedMom.Theta()*TMath::RadToDeg():STData[0].embedMom.Mag()", condition);
  tree.Project("truth", "STData[0].embedMom.Theta()*TMath::RadToDeg():STData[0].embedMom.Mag()");
  truth -> Scale(phase_space_factor);
 
  TEfficiency efficiency(*det, *truth);
  det -> SetDirectory(0);
  truth -> SetDirectory(0);
  efficiency.SetDirectory(0);
  return efficiency;
};

/********************************************************
* Efficiency from conc
*********************************************************/

EfficiencyFromConcFactory::EfficiencyFromConcFactory(){}

void EfficiencyFromConcFactory::SetDataBaseForPDG(int t_pdg, const std::string& t_efficiency_db)
{
  fEfficiencyDB[t_pdg] = t_efficiency_db;
}

void EfficiencyFromConcFactory::CompressFile(const std::string& t_efficiency_db, const std::string& t_compressed_db)
{
  TChain tree("spirit");
  for(const auto& filename : glob(t_efficiency_db.c_str()))
    tree.Add(filename.c_str());

  TFile output(t_compressed_db.c_str(), "RECREATE");
  TTree compressed("spirit", "spirit");

  STData *data = new STData;
  STData *singles = new STData;
  tree.SetBranchAddress("EvtData", &data);
  compressed.Branch("EvtData", singles);

  int n = tree.GetEntries();
  for(int i = 0; i < n; ++i)
  {
    std::cout << "Working on entry " << i << " out of " << n << "\r" << std::flush;
    tree.GetEntry(i);
    singles -> ResetDefaultWithLength(1);
    singles -> multiplicity = data -> multiplicity;
    singles -> vaMultiplicity = data -> vaMultiplicity;
    singles -> tpcVertex = data -> tpcVertex;
    singles -> bdcVertex = data -> bdcVertex;
    singles -> embedMom = data -> embedMom;

    singles -> aoq = data -> aoq;
    singles -> z = singles -> z;
    singles -> a = data -> a;
    singles -> b = data -> b;
    singles -> proja = data -> proja;
    singles -> projb = data -> projb;
    singles -> projx = data -> projx;
    singles -> projy = data -> projy;
    singles -> beamEnergy = data -> beamEnergy;
    singles -> beta = data -> beta;

    singles -> beamEnergyTargetPlane = data -> beamEnergyTargetPlane;
    singles -> betaTargetPlane = data -> betaTargetPlane;

    for(int part = 0; part < data -> multiplicity; ++part)
      if(data -> vaEmbedTag[part])
      {
        singles -> recoMom[0] = data -> recoMom[part];
        singles -> recoPosPOCA[0] = data -> recoPosPOCA[part];
        singles -> recoPosTargetPlane[0] = data -> recoPosTargetPlane[part];
        singles -> recodpoca[0] = data -> recodpoca[part];
        singles -> recoNRowClusters[0] = data -> recoNRowClusters[part];
        singles -> recoNLayerClusters[0] = data -> recoNLayerClusters[part];
        singles -> recoCharge[0] = data -> recoCharge[part];
        singles -> recoEmbedTag[0] = data -> recoEmbedTag[part];
        singles -> recodedx[0] = data -> recodedx[part];

        singles -> vaMom[0] = data -> vaMom[part];
        singles -> vaPosPOCA[0] = data -> vaPosPOCA[part];
        singles -> vaPosTargetPlane[0] = data -> vaPosTargetPlane[part];
        singles -> vadpoca[0] = data -> vadpoca[part];
        singles -> vaNRowClusters[0] = data -> vaNRowClusters[part];
        singles -> vaNLayerClusters[0] = data -> vaNLayerClusters[part];
        singles -> vaCharge[0] = data -> vaCharge[part];
        singles -> vaEmbedTag[0] = data -> vaEmbedTag[part];
        singles -> vadedx[0] = data -> vadedx[part];
      }
    compressed.Fill();
  }

  output.cd();
  compressed.Write();
  std::cout << std::endl;
}

/*
TEfficiency EfficiencyFromConcFactory::FinalizeBins(int t_pdg,
                                                    bool t_verbose)
{
  TChain tree("spirit");
  for(const auto& filename : glob(fEfficiencyDB[t_pdg].c_str()))
    tree.Add(filename.c_str());

  TH2F *det = new TH2F("det", "det", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  TH2F *truth = new TH2F("truth", "truth", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  
  STAnaParticleDB::FillTDatabasePDG();
  int Z = TDatabasePDG::Instance() -> GetParticle(t_pdg) -> Charge()/3;
  if(tree.GetEntries() > 0)
  {
    TString condition = TString::Format("vaEmbedTag && ((embedMom.Mag() - %d*vaMom.Mag())/embedMom.Mag()) < 0.2 && ", Z);
    //TString wrappedPhi = "((vaMom.Phi() < 0)? vaMom.Phi()*TMath::RadToDeg() + 360:vaMom.Phi()*TMath::RadToDeg())";
    tree.SetAlias("phi", "((embedMom.Phi() < 0)? embedMom.Phi()*TMath::RadToDeg() + 360:embedMom.Phi()*TMath::RadToDeg())");
    condition += "(";
    for(int i = 0; i < phi_cut_.size(); ++i)
    {  
      if(i > 0) condition += ") || ";
      condition += TString::Format("(phi > %f && %f > phi", phi_cut_[i].first, phi_cut_[i].second);
    }
    condition += "))";

    condition += TString::Format(" && vaNRowClusters + vaNLayerClusters > %d && recodpoca.Mag() < %f",
                                 num_clusters_, dist_2_vert_);

    tree.Project("det", "embedMom.Theta()*TMath::RadToDeg():embedMom.Mag()", condition);
    tree.Project("truth", "embedMom.Theta()*TMath::RadToDeg():embedMom.Mag()");

    det -> Smooth();
    truth -> Scale(phase_space_factor);
    truth -> Smooth();
  } else  
    std::cout << "No data is loaded for particle " << TDatabasePDG::Instance()->GetParticle(t_pdg)->GetName() << ". Will return an empty efficiency instead." << std::endl; 
 
  TEfficiency efficiency(*det, *truth);
  det -> SetDirectory(0);
  truth -> SetDirectory(0);
  efficiency.SetDirectory(0);
  return efficiency;
};*/

void EfficiencyFromConcFactory::SetUnfoldingDist(TH2F *dist)
{
  if(dist) fUnfoldingDist = (TH2F*) dist -> Clone(TString::Format("%s_cloned", dist->GetName()));
}

TEfficiency EfficiencyFromConcFactory::FinalizeBins(int t_pdg,
                                                    bool t_verbose)
{
  TChain tree("spirit");
  for(const auto& filename : glob(fEfficiencyDB[t_pdg].c_str()))
    tree.Add(filename.c_str());

  TH2F *det = new TH2F("det", "det", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  TH2F *truth = new TH2F("truth", "truth", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  det -> SetStatOverflows(TH1::EStatOverflows::kIgnore);
  truth -> SetStatOverflows(TH1::EStatOverflows::kIgnore);

  
  STAnaParticleDB::FillTDatabasePDG();
  int Z = TDatabasePDG::Instance() -> GetParticle(t_pdg) -> Charge()/3;
  STData *fData = new STData;
  tree.SetBranchAddress("EvtData", &fData);
  int n = tree.GetEntries();
  for(int i = 0; i < n; ++i)
  {
    tree.GetEntry(i);
    const auto& mom = fData -> embedMom;
    double mag = mom.Mag();
    double theta = mom.Theta()*TMath::RadToDeg();
    double phi = mom.Phi()*TMath::RadToDeg();
    double weight = 1;
    if(mom_bin_.min < mag && mag < mom_bin_.max && theta_bin_.min < theta && theta < theta_bin_.max)
      if(fUnfoldingDist) weight = fUnfoldingDist -> Interpolate(mag, theta);
    if(weight == 0) weight = 1;
    if(phi < 0) phi += 360;
    
    // fill numerator
    for(int part = 0; part < fData -> vaEmbedTag.size(); ++part)
      if(fData -> vaEmbedTag[part])
        if(fData -> vaNRowClusters[part] + fData -> vaNLayerClusters[part] > num_clusters_ && 
           fData -> recodpoca[part].Mag() < dist_2_vert_)
        {
          auto& vaMom = fData -> vaMom[part];
          double vaMag = vaMom.Mag()*Z;
          if((vaMag - mag)/mag < 0.2)
            for(const auto& phi_range : phi_cut_) // see if the particle is within phi range
              if(fUnfoldingDist)
              {
                double vaPhi = vaMom.Phi()*TMath::RadToDeg();
                if(vaPhi < 0) vaPhi += 360;
                if(phi_range.first < vaPhi && vaPhi < phi_range.second)
                {
                  {
                    double vaTheta = vaMom.Theta()*TMath::RadToDeg();
                    if(mom_bin_.min < vaMag && vaMag < mom_bin_.max && theta_bin_.min < vaTheta && vaTheta < theta_bin_.max)
                      det -> Fill(vaMag, vaTheta, weight);
                  }
                  break;
                }
              }
              else if(phi_range.first < phi && phi < phi_range.second) 
              {
                det -> Fill(mag, theta, weight);
                break;
              }
        }

    truth -> Fill(mag, theta, weight);
    std::cout << "Processing entry " << i << " out of " << n << "\r" << std::flush;
  }

  std::cout << std::endl;
  det -> Smooth();
  auto temp = this -> UpScaling(det, upscale_factor, upscale_factor);
  det -> Delete();
  det = temp;
  truth -> Scale(phase_space_factor);
  truth -> Smooth();
  temp = this -> UpScaling(truth, upscale_factor, upscale_factor);
  truth -> Delete();
  truth = temp;

  for(int i = 0; i < det -> GetNbinsX() + 2; ++i)
    for(int j = 0; j < det -> GetNbinsY() + 2; ++j)
      if(det -> GetBinContent(i, j) > truth -> GetBinContent(i, j))
        det -> SetBinContent(i, j, truth -> GetBinContent(i, j));
        //std::cout << "Inc " << det -> GetXaxis() -> GetBinCenter(i) << " " << det -> GetYaxis() -> GetBinCenter(i) << " " << det -> GetBinContent(i, j)  << " " << truth -> GetBinContent(i, j) << std::endl;

  if(n == 0)
    std::cout << "No data is loaded for particle " << TDatabasePDG::Instance()->GetParticle(t_pdg)->GetName() << ". Will return an empty efficiency instead." << std::endl; 
 
  TEfficiency efficiency(*det, *truth);
  det -> SetDirectory(0);
  truth -> SetDirectory(0);
  efficiency.SetDirectory(0);
  return efficiency;
};

/********************************************************
* Efficiency from conc
*********************************************************/

EfficiencyInCMFactory::EfficiencyInCMFactory(){}

void EfficiencyInCMFactory::SetDataBaseForPDG(int t_pdg, const std::string& t_efficiency_db)
{
  fEfficiencyDB[t_pdg] = t_efficiency_db;
}

void EfficiencyInCMFactory::TransformBackToCM(const std::string& t_efficiency_db, const std::string& t_cm_db, int fragMass, int targetMass, int charge, double energyPerN)
{
  const Double_t fNucleonMass = 931.5;
  TChain tree("spirit");
  for(const auto& filename : glob(t_efficiency_db.c_str()))
    tree.Add(filename.c_str());

  TFile output(t_cm_db.c_str(), "RECREATE");
  TTree compressed("spirit", "spirit");

  STData *data = new STData;
  STData *singles = new STData;
  tree.SetBranchAddress("EvtData", &data);
  compressed.Branch("EvtData", singles);

  double mass = fragMass*fNucleonMass;
  double mass2 = mass*mass;

  int n = tree.GetEntries();
  for(int i = 0; i < n; ++i)
  {
    std::cout << "Working on entry " << i << " out of " << n << "\r" << std::flush;
    tree.GetEntry(i);
    singles -> ResetDefaultWithLength(1);
    singles -> multiplicity = data -> multiplicity;
    singles -> vaMultiplicity = data -> vaMultiplicity;
    singles -> tpcVertex = data -> tpcVertex;
    singles -> bdcVertex = data -> bdcVertex;

    singles -> aoq = data -> aoq;
    singles -> z = singles -> z;
    singles -> a = data -> a;
    singles -> b = data -> b;
    singles -> proja = data -> proja;
    singles -> projb = data -> projb;
    singles -> projx = data -> projx;
    singles -> projy = data -> projy;
    singles -> beamEnergy = data -> beamEnergy;
    singles -> beta = data -> beta;

    singles -> beamEnergyTargetPlane = data -> beamEnergyTargetPlane;
    singles -> betaTargetPlane = data -> betaTargetPlane;

    // rotate embed mom
    TVector3 beamDirection(TMath::Tan(data -> proja/1000.), TMath::Tan(data ->projb/1000.),1.);
    beamDirection = beamDirection.Unit();
    auto rotationAxis = beamDirection.Cross(TVector3(0,0,1));
    auto rotationAngle = beamDirection.Angle(TVector3(0,0,1));

    int beamMass = (data -> aoq)*(data -> z) + 0.5;
    double EBeam = energyPerN*beamMass + beamMass*fNucleonMass;
    double PBeam = sqrt(EBeam*EBeam - beamMass*beamMass*fNucleonMass*fNucleonMass);
    TLorentzVector LV(0,0,PBeam,EBeam);
    double beta = PBeam/(LV.Gamma()*beamMass*fNucleonMass + targetMass*fNucleonMass);
    auto vBeam = TVector3(0,0,-beta);

    auto TransformVec = [&vBeam, &rotationAngle, &rotationAxis, mass2](TVector3& vec)
    {
      vec.Rotate(rotationAngle, rotationAxis);
      TLorentzVector momLV(vec.x(), vec.y(), vec.z(), sqrt(vec.Mag2() + mass2));
      momLV.Boost(vBeam);
      vec.SetXYZ(momLV.Px(), momLV.Py(), momLV.Pz());
    };
 
    singles -> embedMom = data -> embedMom;
    TransformVec(singles -> embedMom);

    for(int part = 0; part < data -> multiplicity; ++part)
      if(data -> vaEmbedTag[part])
      {
        singles -> recoMom[0] = data -> recoMom[part]*charge;
        TransformVec(singles -> recoMom[0]);
        singles -> recoPosPOCA[0] = data -> recoPosPOCA[part];
        singles -> recoPosTargetPlane[0] = data -> recoPosTargetPlane[part];
        singles -> recodpoca[0] = data -> recodpoca[part];
        singles -> recoNRowClusters[0] = data -> recoNRowClusters[part];
        singles -> recoNLayerClusters[0] = data -> recoNLayerClusters[part];
        singles -> recoCharge[0] = data -> recoCharge[part];
        singles -> recoEmbedTag[0] = data -> recoEmbedTag[part];
        singles -> recodedx[0] = data -> recodedx[part];

        singles -> vaMom[0] = data -> vaMom[part]*charge;
        TransformVec(singles -> vaMom[0]);
        singles -> vaPosPOCA[0] = data -> vaPosPOCA[part];
        singles -> vaPosTargetPlane[0] = data -> vaPosTargetPlane[part];
        singles -> vadpoca[0] = data -> vadpoca[part];
        singles -> vaNRowClusters[0] = data -> vaNRowClusters[part];
        singles -> vaNLayerClusters[0] = data -> vaNLayerClusters[part];
        singles -> vaCharge[0] = data -> vaCharge[part];
        singles -> vaEmbedTag[0] = data -> vaEmbedTag[part];
        singles -> vadedx[0] = data -> vadedx[part];
      }
    compressed.Fill();
  }

  output.cd();
  compressed.Write();
}

void EfficiencyInCMFactory::SetUnfoldingDist(TH2F *dist)
{
  if(dist) fUnfoldingDist = (TH2F*) dist -> Clone(TString::Format("%s_cloned", dist->GetName()));
}

TEfficiency EfficiencyInCMFactory::FinalizeBins(int t_pdg,
                                                bool t_verbose)
{
  TChain tree("spirit");
  for(const auto& filename : glob(fEfficiencyDB[t_pdg].c_str()))
    tree.Add(filename.c_str());

  TH2F *det = new TH2F("det", "det", CMz_bin_.nbins, CMz_bin_.min, CMz_bin_.max, pt_bin_.nbins, pt_bin_.min, pt_bin_.max);
  TH2F *truth = new TH2F("truth", "truth", CMz_bin_.nbins, CMz_bin_.min, CMz_bin_.max, pt_bin_.nbins, pt_bin_.min, pt_bin_.max);
  det -> SetStatOverflows(TH1::EStatOverflows::kIgnore);
  truth -> SetStatOverflows(TH1::EStatOverflows::kIgnore);

  
  STAnaParticleDB::FillTDatabasePDG();
  int Z = TDatabasePDG::Instance() -> GetParticle(t_pdg) -> Charge()/3;
  STData *fData = new STData;
  tree.SetBranchAddress("EvtData", &fData);
  int n = tree.GetEntries();
  for(int i = 0; i < n; ++i)
  {
    tree.GetEntry(i);
    const auto& mom = fData -> embedMom;
    double pt = mom.Perp();
    double z = mom.z();
    double phi = mom.Phi()*TMath::RadToDeg();
    double weight = 1;
    if(CMz_bin_.min < z && z < CMz_bin_.max && pt_bin_.min < pt && pt < pt_bin_.max)
      if(fUnfoldingDist) weight = fUnfoldingDist -> Interpolate((z < 0)? -z : z, pt);
    if(weight == 0) weight = 1;
    if(phi < 0) phi += 360;
    
    // fill numerator
    for(int part = 0; part < fData -> vaEmbedTag.size(); ++part)
      if(fData -> vaEmbedTag[part])
        if(fData -> vaNRowClusters[part] + fData -> vaNLayerClusters[part] > num_clusters_ && 
           fData -> recodpoca[part].Mag() < dist_2_vert_)
          for(const auto& phi_range : phi_cut_) // see if the particle is within phi range
            if(fUnfoldingDist)
            {
              auto& vaMom = fData -> vaMom[part];
              double vaPhi = vaMom.Phi()*TMath::RadToDeg();
              if(vaPhi < 0) vaPhi += 360;
              if(phi_range.first < vaPhi && vaPhi < phi_range.second)
              {
                {
                  double vaZ = vaMom.z();
                  double vaPt = vaMom.Perp();
                  if(CMz_bin_.min < vaZ && vaZ < CMz_bin_.max && pt_bin_.min < vaPt && vaPt < pt_bin_.max)
                    det -> Fill(vaZ, vaPt, weight);
                }
                break;
              }
            }
            else if(phi_range.first < phi && phi < phi_range.second) 
            {
              det -> Fill(z, pt, weight);
              break;
            }

    truth -> Fill(z, pt, weight);
    std::cout << "Processing entry " << i << " out of " << n << "\r" << std::flush;
  }

  std::cout << std::endl;
  det -> Smooth();
  auto temp = this -> UpScaling(det, upscale_factor, upscale_factor);
  det -> Delete();
  det = temp;
  truth -> Scale(phase_space_factor);
  truth -> Smooth();
  temp = this -> UpScaling(truth, upscale_factor, upscale_factor);
  truth -> Delete();
  truth = temp;

  for(int i = 0; i < det -> GetNbinsX() + 2; ++i)
    for(int j = 0; j < det -> GetNbinsY() + 2; ++j)
      if(det -> GetBinContent(i, j) > truth -> GetBinContent(i, j))
        det -> SetBinContent(i, j, truth -> GetBinContent(i, j));

  if(n == 0)
    std::cout << "No data is loaded for particle " << TDatabasePDG::Instance()->GetParticle(t_pdg)->GetName() << ". Will return an empty efficiency instead." << std::endl; 
 
  TEfficiency efficiency(*det, *truth);
  det -> SetDirectory(0);
  truth -> SetDirectory(0);
  efficiency.SetDirectory(0);
  return efficiency;
};
