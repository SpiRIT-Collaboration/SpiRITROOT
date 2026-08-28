#include "STSpaceChargeCorrectionTask.hh"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

namespace {
Long64_t gSCDebugTpcEventNum = -1;
Double_t gSCDebugLocalRate = -1.0;
Double_t gSCDebugSheetDensity = 0.0;
Int_t gSCDebugUseNominalBFieldInEOM = 0;
std::vector<TVector3> gSCDebugRawDispMM;
std::vector<Double_t> gSCDebugYAppliedMM;
std::vector<Double_t> gSCDebugYModelMM;
std::vector<Int_t> gSCDebugRawWasClipped;
} // namespace

extern "C" Long64_t STSCGetDebugTpcEventNum() { return gSCDebugTpcEventNum; }
extern "C" Double_t STSCGetDebugLocalRate() { return gSCDebugLocalRate; }
extern "C" Double_t STSCGetDebugSheetDensity() { return gSCDebugSheetDensity; }
extern "C" Int_t STSCGetDebugUseNominalBFieldInEOM() { return gSCDebugUseNominalBFieldInEOM; }
extern "C" Int_t STSCGetDebugRawDisplacementCount() { return static_cast<Int_t>(gSCDebugRawDispMM.size()); }
extern "C" Int_t STSCGetDebugYDisplacementAt(Int_t idx, Double_t* yApplied, Double_t* yModel)
{
  if (idx < 0 || idx >= static_cast<Int_t>(gSCDebugYAppliedMM.size()))
    return 0;
  if (yApplied) *yApplied = gSCDebugYAppliedMM[idx];
  if (yModel) *yModel = (idx < static_cast<Int_t>(gSCDebugYModelMM.size())) ? gSCDebugYModelMM[idx] : gSCDebugYAppliedMM[idx];
  return 1;
}
extern "C" Int_t STSCGetDebugRawDisplacementAt(Int_t idx, Double_t* dx, Double_t* dy, Double_t* dz, Int_t* clipped)
{
  if (idx < 0 || idx >= static_cast<Int_t>(gSCDebugRawDispMM.size()))
    return 0;
  const TVector3& v = gSCDebugRawDispMM[idx];
  if (dx) *dx = v.X();
  if (dy) *dy = v.Y();
  if (dz) *dz = v.Z();
  if (clipped) *clipped = (idx < static_cast<Int_t>(gSCDebugRawWasClipped.size())) ? gSCDebugRawWasClipped[idx] : 0;
  return 1;
}

// Root class headers
#include "FairRun.h"
#include "FairRuntimeDb.h"

ClassImp(STSpaceChargeCorrectionTask);

STSpaceChargeCorrectionTask::STSpaceChargeCorrectionTask() : fIsDrift(kTRUE)
{
  LOG(DEBUG) << "Defaul Constructor of STSpaceChargeCorrectionTask" << FairLogger::endl;
}

STSpaceChargeCorrectionTask::~STSpaceChargeCorrectionTask()
{ LOG(DEBUG) << "Destructor of STSpaceChargeCorrectionTask" << FairLogger::endl; }

InitStatus 
STSpaceChargeCorrectionTask::Init()
{
  if(STRecoTask::Init()==kERROR)
    return kERROR;

  LOG(DEBUG) << "Initilization of STSpaceChargeCorrectionTask" << FairLogger::endl;
  fHitClusterArray = (TClonesArray*) fRootManager->GetObject("STHitCluster");

  if(fUseLocalRate)
    fAuxHeader = (STAuxHeader *) fRootManager -> GetObject("STAuxHeader");

  TPCx = fDigiPar->GetPadPlaneX();
  TPCz = fDigiPar->GetPadPlaneZ();
  this -> SetTPCSize(TPCx, TPCz, TPCy);

  const double EField = 132.7; // 127.4; // V/cm
  const double BField = 0.5; // T
  const double driftVelocity = fUseExternalDriftVelocity ? fDriftVelocity : fDigiPar->GetDriftVelocity();
  this -> InferDriftParameters(driftVelocity, EField, BField);
  if(fIsDrift) this -> CalculateEDrift(driftVelocity, true);
  else LOG(INFO) << "Space Chrage displacement is disabled" << FairLogger::endl;
  return kSUCCESS;
}


void STSpaceChargeCorrectionTask::Exec(Option_t* option)
{
  LOG(DEBUG) << "Exec of STSpaceChargeCorrectionTask" << FairLogger::endl;

  gSCDebugRawDispMM.clear();
  gSCDebugYAppliedMM.clear();
  gSCDebugYModelMM.clear();
  gSCDebugRawWasClipped.clear();
  gSCDebugTpcEventNum = -1;
  gSCDebugLocalRate = -1.0;
  gSCDebugUseNominalBFieldInEOM = this->GetUseNominalBFieldInEOM() ? 1 : 0;

  if(fIsDrift)
  {
    if(fUseLocalRate && fAuxHeader != nullptr && fRateHist != nullptr)
    {
      auto eventNum = fAuxHeader->GetTpcEventNum();
      gSCDebugTpcEventNum = eventNum;

      double rate = 0.0;
      if(eventNum > fRateHist->GetBinLowEdge(fRateHist->GetNbinsX()))
        rate = fRateHist->GetBinContent(fRateHist->GetNbinsX());
      else
        rate = fRateHist->GetBinContent(fRateHist->FindBin(eventNum));
      gSCDebugLocalRate = rate;

      if(!fFirstEventDone || eventNum % fEventFrequency == 0)
      {
        auto density = fDensityScaleSlope * rate * rate + fDensityScaleInter * rate;
        if(density >= 0)
        {
          SetSheetChargeDensity(density);
          UpdateEDrift();
        }
        fFirstEventDone = true;
      }
    }

    gSCDebugSheetDensity = this->GetSheetChargeDensity();

    int nClusters = fHitClusterArray->GetEntries();
    gSCDebugRawDispMM.reserve(nClusters);
    gSCDebugYAppliedMM.reserve(nClusters);
    gSCDebugYModelMM.reserve(nClusters);
    gSCDebugRawWasClipped.reserve(nClusters);

    for(int iCluster = 0; iCluster < nClusters; ++iCluster)
    {
      auto cluster = static_cast<STHitCluster*>(fHitClusterArray->At(iCluster));
      double orig_x = cluster->GetX()/10; // convert mm to cm
      double orig_y = cluster->GetY()/10; // convert mm to cm
      double orig_z = cluster->GetZ()/10; // convert mm to cm
      double new_x, new_y, new_z;
      this->DisplaceElectrons(orig_x, orig_y, orig_z, new_x, new_y, new_z);

      const double model_dy_cm = this->GetYDisplacementModel(orig_x, orig_y, orig_z);
      const double raw_dx_mm = (new_x - orig_x) * 10.0;
      const double raw_dy_model_mm = model_dy_cm * 10.0;
      const double raw_dz_mm = (new_z - orig_z) * 10.0;
      gSCDebugRawDispMM.emplace_back(raw_dx_mm, raw_dy_model_mm, raw_dz_mm);

      const double y_applied_mm = (new_y - orig_y) * 10.0;
      gSCDebugYAppliedMM.emplace_back(y_applied_mm);
      gSCDebugYModelMM.emplace_back(raw_dy_model_mm);

      // convert cm back to mm
      new_x *= 10;
      new_y *= 10;
      new_z *= 10;

      Bool_t wasClipped = kFALSE;
      if(new_x < -TPCx/2.) {
        cluster->SetX(-TPCx/2.);
        wasClipped = kTRUE;
      }
      else if(new_x > TPCx/2.) {
        cluster->SetX(TPCx/2.);
        wasClipped = kTRUE;
      }
      else cluster->SetX(new_x);

      if(fApplyYCorrection) cluster->SetY(new_y);

      if(new_z < 0) {
        cluster->SetZ(0);
        wasClipped = kTRUE;
      }
      else if(new_z >= TPCz) {
        cluster->SetZ(TPCz - 1);
        wasClipped = kTRUE;
      }
      else cluster->SetZ(new_z);

      gSCDebugRawWasClipped.emplace_back(wasClipped ? 1 : 0);
    }
    LOG(INFO) << Space() << "Shift Clusters for space-charge effect" << FairLogger::endl;
  }
  else
  {
    gSCDebugSheetDensity = this->GetSheetChargeDensity();
  }
}

void STSpaceChargeCorrectionTask::SetVerbose(Bool_t value) { fVerbose = value; }
void STSpaceChargeCorrectionTask::SetElectronDrift(Bool_t value) { fIsDrift = value; }
void STSpaceChargeCorrectionTask::SetDriftVelocity(Double_t value)
{
  fDriftVelocity = value;
  fUseExternalDriftVelocity = kTRUE;
}
void STSpaceChargeCorrectionTask::SetApplyYCorrection(Bool_t value) { fApplyYCorrection = value; }
bool STSpaceChargeCorrectionTask::SearchForRunPar(const std::string& filename, int run_num)
{
  // RunInfo.dat in parameters folder should contains information about a run
  // return false if the run is not found
  // return true otherwise

  LOG(INFO) << "Searching space-charge information for run " << run_num << " in " << filename << FairLogger::endl;

  std::ifstream config(filename.c_str());
  if(!config.is_open())
  {
    LOG(ERROR) << "File " << filename << " cannot be opened" << FairLogger::endl;
    return false;
  }

  // line variables to store file content line by line
  std::string line;
  // get rid of the header
  std::getline(config, line);
  while(std::getline(config, line))
  {
    int read_run_num;
    double sheet_charge;
    std::string beam_type;

    std::stringstream ss(line);
    if((ss >> read_run_num >> sheet_charge >> beam_type))
    {
      if(read_run_num == run_num)
      {
        LOG(INFO) << "Loading run info for " << run_num << " from " << filename << FairLogger::endl;

        STSpaceCharge::Projectile proj;
        if(beam_type == "Sn108") proj = STSpaceCharge::Projectile::Sn108;
        else if(beam_type == "Sn112") proj = STSpaceCharge::Projectile::Sn112;
        else if(beam_type == "Sn124") proj = STSpaceCharge::Projectile::Sn124;
        else if(beam_type == "Sn132") proj = STSpaceCharge::Projectile::Sn132;
        else
        {
          LOG(ERROR) << "BeamType " << beam_type << " is not supported" << FairLogger::endl;
          return false;
        }

        this->SetDriftParameters(-4.3556e4,-2.18);
        this->SetProjectile(proj);
        this->SetSheetChargeDensity(sheet_charge);           
        return true;
      }
    }
    else
    {
      LOG(ERROR) << "Cannot parse the following line from " << filename << FairLogger::endl;
      LOG(ERROR) << line << FairLogger::endl;
    }
  }
  // if the class find nothing in the file
  // it will exit the loop when it reaches the end of the file
  LOG(INFO) << "Run " << run_num << " is not found in file " << filename << FairLogger::endl;
  return false;
}

void STSpaceChargeCorrectionTask::SetLocalRate(Double_t scale_s, Double_t scale_i, TString filename, TString histname, Int_t frequency)
{
  fDensityScaleSlope = scale_s;
  fDensityScaleInter = scale_i;
  fEventFrequency = frequency;
  TFile *ratefile = new TFile(filename.Data());
  fRateHist = (TH1D *)ratefile -> Get(histname.Data());
  fUseLocalRate = true;
}

void STSpaceChargeCorrectionTask::UpdateEDrift()
{
   const double driftVelocity = fUseExternalDriftVelocity ? fDriftVelocity : fDigiPar->GetDriftVelocity();
   if(fIsDrift) this -> CalculateEDrift(driftVelocity, true);
}


