#include "STSpaceChargeCorrectionTask.hh"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>

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

  TPCx = fDigiPar->GetPadPlaneX();
  TPCz = fDigiPar->GetPadPlaneZ();
  this -> SetTPCSize(TPCx, TPCz, TPCy);

  const double EField = 127.4; // V/cm
  const double BField = 0.5; // T
  this -> InferDriftParameters(fDigiPar->GetDriftVelocity(), EField, BField);
  if(fIsDrift) this -> CalculateEDrift(fDigiPar->GetDriftVelocity(), true);
  else LOG(INFO) << "Space Chrage displacement is disabled" << FairLogger::endl;
  return kSUCCESS;
}


void STSpaceChargeCorrectionTask::Exec(Option_t* option)
{
  LOG(DEBUG) << "Exec of STSpaceChargeCorrectionTask" << FairLogger::endl;
  if(fIsDrift)
  {
    int nClusters = fHitClusterArray->GetEntries();
    for(int iCluster = 0; iCluster < nClusters; ++iCluster) 
    {
      auto cluster = static_cast<STHitCluster*>(fHitClusterArray -> At(iCluster));
      double orig_x = cluster->GetX()/10; // convert mm to cm
      double orig_y = cluster->GetY()/10; // convert mm to c;
      double orig_z = cluster->GetZ()/10; // convert mm to c;
      double new_x, new_y, new_z; 
      this -> DisplaceElectrons(orig_x, orig_y, orig_z, new_x, new_y, new_z);
      // convert cm back to mm
      new_x *= 10;
      new_y *= 10;
      new_z *= 10;

      if(new_x < -TPCx/2.) cluster -> SetX(-TPCx/2.);
      else if(new_x > TPCx/2.) cluster -> SetX(TPCx/2.);
      else cluster -> SetX(new_x);
      
      if(new_z < 0) cluster -> SetZ(0);
      if(new_z >= TPCz) cluster -> SetZ(TPCz - 1);
      else cluster -> SetZ(new_z);
    }
    LOG(INFO) << Space() << "Shift Clusters for space-charge effect" << FairLogger::endl;
  }
}

void STSpaceChargeCorrectionTask::SetVerbose(Bool_t value) { fVerbose = value; }
void STSpaceChargeCorrectionTask::SetElectronDrift(Bool_t value) { fIsDrift = value; }
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


