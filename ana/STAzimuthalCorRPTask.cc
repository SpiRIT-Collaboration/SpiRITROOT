#include "STAzimuthalCorRPTask.hh"
#include "STAnaParticleDB.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TDatabasePDG.h"
#include "TParameter.h"

ClassImp(STAzimuthalCorRPTask);

STAzimuthalCorRPTask::STAzimuthalCorRPTask() 
{
  fLogger = FairLogger::GetLogger(); 
  fSupportedPDG = STAnaParticleDB::SupportedPDG;
}

STAzimuthalCorRPTask::~STAzimuthalCorRPTask()
{}

void STAzimuthalCorRPTask::SetParContainers()
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

InitStatus STAzimuthalCorRPTask::Init()
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

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fFragRapidity = (TClonesArray*) ioMan -> GetObject("FragRapidity");
  fBeamRapidity = (STVectorF*) ioMan -> GetObject("BeamRapidity");

  fReactionPlane = new STVectorF();
  fReactionPlane -> fElements.push_back(0);
  ioMan -> Register("RP", "ST", fReactionPlane, fIsPersistence);

  fReactionPlaneV2 = new STVectorF();
  fReactionPlaneV2 -> fElements.push_back(0);
  ioMan -> Register("RPV2", "ST", fReactionPlaneV2, fIsPersistence);

  fV1RPAngle = new TClonesArray("STVectorF");
  fPhiEff = new TClonesArray("STVectorF");
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    new((*fV1RPAngle)[i]) STVectorF();
    new((*fPhiEff)[i]) STVectorF();
  }
  ioMan -> Register("V1RPAngle", "ST", fV1RPAngle, fIsPersistence);
  ioMan -> Register("PhiEff", "ST", fPhiEff, fIsPersistence);

  return kSUCCESS;
}


void STAzimuthalCorRPTask::Exec(Option_t *opt)
{
  auto beamRap = fBeamRapidity -> fElements[1];
  auto data = static_cast<STData*>(fData -> At(0));
  int num_pdgs = fSupportedPDG.size();
  std::vector<std::vector<double>> X2_elements(num_pdgs), Y2_elements(num_pdgs),
                                   XY_elements(num_pdgs), weight(num_pdgs);
  double Qx = 0;
  int mult = 0;
  for(int i = 0; i < num_pdgs; ++i)
  {
    auto& phiEff = static_cast<STVectorF*>(fPhiEff -> At(i)) -> fElements;
    phiEff.clear();
    auto cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
    auto prob = static_cast<STVectorF*>(fProb -> At(i));
    auto rap = static_cast<STVectorF*>(fFragRapidity -> At(i));
    mult = cmVector -> fElements.size();
    int pdg = fSupportedPDG[i];
    auto TEff = fEff[pdg];

    for(int j = 0; j < mult; ++j) 
    {
      auto vec_unit = cmVector -> fElements[j].XYvector();
      X2_elements[i].push_back(vec_unit.X()*vec_unit.X());
      Y2_elements[i].push_back(vec_unit.Y()*vec_unit.Y());
      XY_elements[i].push_back(vec_unit.X()*vec_unit.Y());

      double eff = 1;
      double theta = cmVector -> fElements[j].Theta();
      double phi = cmVector -> fElements[j].Phi();
      if(TEff && !std::isnan(theta)) eff = TEff -> Interpolate(theta, phi);
      phiEff.push_back(eff);
      //weight[i].push_back(prob -> fElements[j]);
      if(data -> vaNRowClusters[j] + data -> vaNLayerClusters[j] >= fMinNClusters && 
         data -> recodpoca[j].Mag() < fMaxDPOCA && eff > 0 && fabs(2*rap -> fElements[j]/beamRap) > 0.2) 
      {
        weight[i].push_back(prob -> fElements[j]/eff);
        if(rap -> fElements[j] > 0) Qx += weight[i].back()*cmVector -> fElements[j].y();
        else Qx -= weight[i].back()*cmVector -> fElements[j].x();
      } 
      else weight[i].push_back(0);
    }
  }

  double X2 = 0, Y2 = 0, XY = 0;
  for(int i = 0; i < num_pdgs; ++i)
    for(int j = 0; j < mult; ++j)
    {
      X2 += X2_elements[i][j]*weight[i][j];
      Y2 += Y2_elements[i][j]*weight[i][j];
      XY += XY_elements[i][j]*weight[i][j];
    }

  double a1 = (Y2 - X2)/(2*XY);
  double a2 = sqrt((X2 - Y2)*(X2 - Y2) + 4*XY*XY)/(2*XY);
  double a_plus = a1 + a2;
  double a_minus = a1 - a2;
  
  // find whcih root minimize D2
  double d2_plus = 0, d2_minus = 0;
  for(int j = 0; j < mult; ++j)
    for(int i = 0; i < num_pdgs; ++i)
    {
      auto cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
      auto vec_unit = cmVector -> fElements[j].XYvector();
      d2_plus += (X2_elements[i][j] + Y2_elements[i][j] - 
                 (vec_unit.X() + vec_unit.Y()*a_plus)*
                 (vec_unit.X() + vec_unit.Y()*a_plus)/(1 + a_plus*a_plus))*
                 weight[i][j];
      d2_minus += (X2_elements[i][j] + Y2_elements[i][j] - 
                  (vec_unit.X() + vec_unit.Y()*a_minus)*
                  (vec_unit.X() + vec_unit.Y()*a_minus)/(1 + a_minus*a_minus))*
                  weight[i][j];
    }

  double slope = (d2_minus < d2_plus)? a_minus : a_plus;
  //slope = -1/slope;
  auto wrap_angle_to_range = [Qx](double rpAngle)
    {
      //rpAngle += TMath::Pi()/2;
      if(Qx < 0)  rpAngle -= TMath::Pi();
      rpAngle += TMath::Pi()/2.;
      return rpAngle;
    };

  fReactionPlane -> fElements[0] = wrap_angle_to_range(atan(slope));
  fReactionPlaneV2 -> fElements[0] = fReactionPlane -> fElements[0];

  for(int i = 0; i < num_pdgs; ++i)
    static_cast<STVectorF*>(fV1RPAngle -> At(i)) -> fElements.clear();

  for(int i = 0; i < num_pdgs; ++i)
    for(int j = 0; j < mult; ++j)
    {
      double X2_part = X2 - X2_elements[i][j]*weight[i][j];
      double Y2_part = Y2 - Y2_elements[i][j]*weight[i][j];
      double XY_part = XY - XY_elements[i][j]*weight[i][j];
      double a1_part = (Y2_part - X2_part)/(2*XY_part);
      double a2_part = sqrt((X2_part - Y2_part)*(X2_part - Y2_part) + 4*XY_part*XY_part)/(2*XY_part);
      double slope_part = (d2_minus < d2_plus)? a1_part - a2_part : a1_part + a2_part;
      static_cast<STVectorF*>(fV1RPAngle -> At(i)) -> fElements.push_back(wrap_angle_to_range(atan(slope_part)));
    }
}

void STAzimuthalCorRPTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STAzimuthalCorRPTask::SetCutConditions(int min_clusters, double max_dpoca)
{ fMinNClusters = min_clusters; fMaxDPOCA = max_dpoca; }
void STAzimuthalCorRPTask::LoadPhiEff(const std::string& eff_filename)
{ fEffFilename = eff_filename; }

