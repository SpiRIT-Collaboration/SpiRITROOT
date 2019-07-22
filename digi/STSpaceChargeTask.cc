#include "STSpaceChargeTask.hh"
#include "STProcessManager.hh"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>

// Root class headers
#include "TLorentzVector.h"
#include "TString.h"
#include "TRandom.h"
#include "TError.h"

EquationOfMotion::EquationOfMotion(const FieldFunc& t_EField,
                                     const FieldFunc& t_BField,
                                     double t_mu,
                                     double t_wtau) : 
  EField_(t_EField), BField_(t_BField), mu_(t_mu), wtau_(t_wtau){};

TVector3 EquationOfMotion::operator()(const TVector3& t_pos, double t_t)
{
  /*********************************
  * Equation of Motion is
  * dx/dt = mu/(1+wtau^2)*(E + wtau/|B|(E X B) + wtau^2 (E.B)B/B^2
  ***********************************/
  double factor1 = mu_/(1+wtau_*wtau_);
  auto E = this->EField_(t_pos); // very bad variable naming scheme
  auto B = this->BField_(t_pos); // but who cares
  TVector3 factor2 = E + wtau_/B.Mag()*(E.Cross(B)) + ((wtau_*wtau_*E.Dot(B))/(B.Mag2()))*B;
  return factor1*factor2;
}

TVector3 RK4Stepper(EquationOfMotion& t_eom, const TVector3& t_pos, double t_t, double t_dt)
{
  /********************
  * Runge-Kutta solver
  * y_n+1 = y_n + 1/6*(k1 + 2k2 + 2k3 + k4)
  * Reference: https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
  *********************/
  TVector3 k1 = t_dt*t_eom(t_pos, t_t);
  TVector3 k2 = t_dt*t_eom(t_pos + 0.5*k1, t_t + t_dt/2);
  TVector3 k3 = t_dt*t_eom(t_pos + 0.5*k2, t_t + t_dt/2);
  TVector3 k4 = t_dt*t_eom(t_pos + k3, t_t + t_dt);

  return t_pos + 1/6.*(k1 + 2*k2 + 2*k3 + k4);
}

ElectronDrifter::ElectronDrifter(double t_dt, EquationOfMotion& t_eom) : 
  dt_(t_dt),   
  eom_(t_eom),
  stop_cond_([](const TVector3& t_pos, double t){ return t_pos.Y() > 0; }) 
 {};

ElectronDrifter& ElectronDrifter::DriftUntil(const std::function<bool(const TVector3&, double)>& t_stop_cond)
{ stop_cond_ = t_stop_cond; return *this; }

TVector3 ElectronDrifter::DriftFrom(const TVector3& t_pos)
{
  t_ = 0;
  TVector3 pos = t_pos;
  while(!stop_cond_(pos, t_))
  {
    double old_y = pos.Y();
    pos = RK4Stepper(eom_, pos, t_, dt_);
    t_ += dt_;
  }
  return pos;
};

double ElectronDrifter::GetDriftTime() { return t_; };


STSpaceChargeTask::STSpaceChargeTask()
:FairTask("STSpaceChargeTask"),
 fEventID(0),
 fIsDrift(kTRUE),
 fIsPersistence(kTRUE),
 fBField(nullptr),
 fDispX(nullptr),
 fDispY(nullptr),
 fDispZ(nullptr),
 fOffsetX(0),
 fOffsetY(0),
 fOffsetZ(0),
 fRotateXZ(0),
 fmu(-4.252e4),
 fwtau(-4)
{
  fIsPersistence = kFALSE;
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STSpaceChargeTask");

  // Default value
  std::string vmc_dir = std::getenv("VMCWORKDIR");
  fEFieldFile = vmc_dir + "/input/E-field.root";
  fProj = "132Sn";
  fBeamRate = 3.14e-8;
}

STSpaceChargeTask::~STSpaceChargeTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STSpaceChargeTask");
  if(fEx) fEx->Delete();
  if(fEy) fEy->Delete();
  if(fEz) fEz->Delete();
}

void 
STSpaceChargeTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STSpaceChargeTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STSpaceChargeTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STSpaceChargeTask");

  FairRootManager* ioman = FairRootManager::Instance();

  fMCPointArray = (TClonesArray*) ioman->GetObject("STMCPoint");
  fDispMCPointArray = new TClonesArray("STMCPoint");
  ioman->Register("DispMCPoint","ST",fDispMCPointArray,fIsPersistence);

  if(fIsDrift)
  {
    if(fDispX && fDispY && fDispZ)
      fLogger->Info(MESSAGE_ORIGIN,"Using custom displacement map");
    else
      this->CalculateEDrift(fPar->GetDriftVelocity());
  } else fLogger->Info(MESSAGE_ORIGIN, "Space Chrage displacement is disabled");
  return kSUCCESS;
}

void STSpaceChargeTask::fSetEField()
{
  fLogger->Info(MESSAGE_ORIGIN,("Loading E-field solution from " + fEFieldFile + " for projectile " + fProj).c_str());
  TFile file(fEFieldFile.c_str());
  if(!file.IsOpen()) fLogger->Fatal(MESSAGE_ORIGIN, "E-field file cannot be opened!");
  auto homox = static_cast<TH3D*>(file.Get(("homo_" + fProj + "_Ex").c_str()));
  auto homoy = static_cast<TH3D*>(file.Get(("homo_" + fProj + "_Ey").c_str()));
  auto homoz = static_cast<TH3D*>(file.Get(("homo_" + fProj + "_Ez").c_str()));

  auto nohomox = static_cast<TH3D*>(file.Get(("nohomo_" + fProj + "_Ex").c_str()));
  auto nohomoy = static_cast<TH3D*>(file.Get(("nohomo_" + fProj + "_Ey").c_str()));
  auto nohomoz = static_cast<TH3D*>(file.Get(("nohomo_" + fProj + "_Ez").c_str()));

  fLogger->Info(MESSAGE_ORIGIN,TString::Format("E-field is calculated with beam rate = %e", fBeamRate));
  double factor = fBeamRate / 3.14e-8; //3.14e-8 is the beam rate of run 2841
  if(homox && homoy && homoz && nohomox && nohomoy && nohomoz)
  {
    homox->Add((nohomox->Scale(factor), nohomox));
    homoy->Add((nohomoy->Scale(factor), nohomoy));
    homoz->Add((nohomoz->Scale(factor), nohomoz));

    fEx = static_cast<TH3D*>(homox->Clone("shiftX"));
    fEy = static_cast<TH3D*>(homoy->Clone("shiftY"));
    fEz = static_cast<TH3D*>(homoz->Clone("shiftZ"));
    fEx -> SetDirectory(0); // disable memory management for this histogram
    fEy -> SetDirectory(0);
    fEz -> SetDirectory(0);
  }
  else fLogger->Fatal(MESSAGE_ORIGIN, "E-field cannot be loaded from files.");
}

void STSpaceChargeTask::CalculateEDrift(double drift_vel, bool t_invert)
{
  if(!fBField) fLogger->Fatal(MESSAGE_ORIGIN, "B-field is not loaded");
  else fBField->Init();
  gErrorIgnoreLevel = kFatal;
  fLogger->Info(MESSAGE_ORIGIN, "Begin calculation of electron displacement map");
  this -> fSetEField();

  // Rewrite E and B field in a form that can be read by ElectronDrifter
  auto EFieldWrapper = this->GetEFieldWrapper();
  auto BFieldWrapper = this->GetBFieldWrapper();
  auto eom = EquationOfMotion(EFieldWrapper, BFieldWrapper, fmu, fwtau); 
  const double dt = (t_invert)? -2e-7:2e-7; // seconds
  auto drifter = ElectronDrifter(dt, eom);

  // mu= -4.252e4, wtau= -4 was found to reproduce E cross B result
  // still subjected to change, not finalized
  if(t_invert) fLogger->Info(MESSAGE_ORIGIN, "Dispacemant is inverted, i.e. arguments of the displacement map is the detected locattion");
  else fLogger->Info(MESSAGE_ORIGIN, "Displacement is not inverted, i.e. arguments of the displacement map is the physical location");
  fLogger->Info(MESSAGE_ORIGIN, TString::Format("Drift parameter: mu = %f, wtau = %f", fmu, fwtau));
  fLogger->Info(MESSAGE_ORIGIN, TString::Format("Displacement map is shifted by: x = %f, y = %f, z = %f", fOffsetX, fOffsetY, fOffsetZ));
  fLogger->Info(MESSAGE_ORIGIN, TString::Format("E field is rotated in the XZ plane by %f deg", fRotateXZ*180/M_PI));
  // create displacement map with the following dimensions:
  // all dimensions are in cm

  double x_min = -50, x_max = 50;
  double y_min = -55, y_max = 0;
  double z_min = 0, z_max = 150;
  int binsx = 40, binsy = 40, binsz = 30;

  fDispX = new TH3D("shiftX_data","shiftX",binsx,x_min,x_max,binsy,y_min,y_max,binsz,z_min,z_max);
  fDispY = new TH3D("shiftY_data","shiftY",binsx,x_min,x_max,binsy,y_min,y_max,binsz,z_min,z_max);
  fDispZ = new TH3D("shiftZ_data","shiftZ",binsx,x_min,x_max,binsy,y_min,y_max,binsz,z_min,z_max);
   

  int counter = 1;
  int percentage = 0;
  if(fCustomRule) fLogger->Info(MESSAGE_ORIGIN, "Calculating displcement map with custom rule");
  std::cout << "STSpaceChargeTask Displacement map Calculation Percent completion: " << "\t";
  for(int idx = 1; idx <= binsx; ++idx)
   for(int idy = 1; idy <= binsy; ++idy)
     for(int idz = 1; idz <= binsz; ++idz)
     {
       double x = fDispX->GetXaxis()->GetBinCenter(idx);
       double y = fDispX->GetYaxis()->GetBinCenter(idy);
       double z = fDispX->GetZaxis()->GetBinCenter(idz);

       // initial position
       double diffx;
       double diffy;
       double diffz;
      
       if(fCustomRule) 
         fCustomRule(x, y, z, diffx, diffy, diffz);
       else 
       {
         TVector3 pos;
         if(t_invert)
         {
           pos[0] = x; pos[1] = 0; pos[2] = z;
           pos = drifter.DriftUntil([drift_vel, y](const TVector3& t_pos, double t){ return t*1e6*drift_vel < y; })
                        .DriftFrom(pos);
         }else
         {
           pos[0] = x; pos[1] = y; pos[2] = z;
           pos = drifter.DriftFrom(pos);
         }
         diffx = pos.X()-x;
         diffy = 0;
         diffz = pos.Z()-z ;
       }
       fDispX->SetBinContent(idx, idy, idz, diffx);
       fDispY->SetBinContent(idx, idy, idz, diffy);// still not sure about how drift velocity is calculated. Recommend that we disable y drift for now (-drift_vel*drifter.GetDriftTime()*1e6-y));
       fDispZ->SetBinContent(idx, idy, idz, diffz);
       int new_progress =  int(counter/(double) (binsx*binsy*binsz)*100);
       if(new_progress != percentage)
       {
         percentage = new_progress;
         std::cout << "\b\b\b\b\b" << std::setw(3) << percentage << " %" << std::flush;
       }
       ++counter;
     }
  std::cout << std::endl;
}

void 
STSpaceChargeTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STSpaceChargeTask");
  if(!fDispMCPointArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No Drifted MC Point Array!");
  fDispMCPointArray -> Delete();
  Int_t nMCPoints = fMCPointArray->GetEntries();
  /**
   * NOTE! that fMCPoint has unit of [cm] for length scale,
   * [GeV] for energy and [ns] for time.
   */
  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    Int_t index = fDispMCPointArray->GetEntriesFast();
    STMCPoint *drifted_mc_point
      = new ((*fDispMCPointArray)[index])
        STMCPoint(*fMCPoint);
    double posx, posy, posz;
    if(fIsDrift)
    {
      this -> fSpaceChargeEffect(fMCPoint->GetX(), fMCPoint->GetY(), fMCPoint->GetZ(), posx, posy, posz);
      drifted_mc_point -> SetXYZ(posx, posy, posz);
    }
  }

  if(fIsDrift)
    fLogger->Info(MESSAGE_ORIGIN, 
              Form("Event #%d : MC points (%d) found. They are dispaced due to space charge",
                   fEventID++, nMCPoints));

  return;
}

void STSpaceChargeTask::SetCustomMap(TH3D* dispx, TH3D* dispy, TH3D* dispz)
{ fDispX = dispx; fDispY = dispy; fDispZ = dispz; }
void STSpaceChargeTask::SetBField(FairField* Bfield){ fBField = Bfield; }
void STSpaceChargeTask::SetEFieldSolution(const std::string& value)
{
  std::string vmc_dir(std::getenv("VMCWORKDIR")); 
  fEFieldFile = vmc_dir + "/input/" + value; 
}

void STSpaceChargeTask::SetBeamRate(Double_t value){ fBeamRate = value; }
void STSpaceChargeTask::SetProjectile(const std::string& value){ fProj = value; }
void STSpaceChargeTask::SetDriftParameters(double mu, double wtau) { fmu = mu; fwtau = wtau; }
void STSpaceChargeTask::SetCustomRule(const std::function<void(double, double, double, double&, double&, double&)>& rule) { fCustomRule = rule; };

void STSpaceChargeTask::SetPersistence(Bool_t value) { fIsPersistence = value; }
void STSpaceChargeTask::SetVerbose(Bool_t value) { fVerbose = value; }
void STSpaceChargeTask::SetElectronDrift(Bool_t value) { fIsDrift = value; }
void STSpaceChargeTask::fSpaceChargeEffect(double x, double y, double z, 
                       double& x_out, double& y_out, double& z_out)
{
  // remember the unit is now in cm
  x_out = x + fDispX->Interpolate(x + fOffsetX, y + fOffsetY, z + fOffsetZ);
  y_out = y + fDispY->Interpolate(x + fOffsetX, y + fOffsetY, z + fOffsetZ); 
  z_out = z + fDispZ->Interpolate(x + fOffsetX, y + fOffsetY, z + fOffsetZ);

}

std::function<TVector3(const TVector3&)> STSpaceChargeTask::GetEFieldWrapper()
{
  return [this](const TVector3& t_pos)
    {
      double x = t_pos.X();
      double y = t_pos.Y();
      double z = t_pos.Z();
      // rotate x and z
      double rot_x = x*cos(fRotateXZ) - z*sin(fRotateXZ);
      double rot_z = x*sin(fRotateXZ) + z*cos(fRotateXZ);
      double ex = fEx->Interpolate(rot_x, y, rot_z);
      double ey = fEy->Interpolate(rot_x, y, rot_z);
      double ez = fEz->Interpolate(rot_x, y, rot_z);
      double rot_ex = ex*cos(fRotateXZ) + ez*sin(fRotateXZ);
      double rot_ez = -ex*sin(fRotateXZ) + ez*cos(fRotateXZ);
      if(ey >= -10) ey = -127.7;
      return TVector3(rot_ex, ey, rot_ez);
    };
}

std::function<TVector3(const TVector3&)> STSpaceChargeTask::GetBFieldWrapper()
{
  return [this](const TVector3& t_pos)
    {
      double x = t_pos.X();
      double y = t_pos.Y();
      double z = t_pos.Z();
      return TVector3(fBField->GetBx(x,y,z)/10,
                      fBField->GetBy(x,y,z)/10,
                      fBField->GetBz(x,y,z)/10);
    };
}


void STSpaceChargeTask::RotateEFieldXZDeg(double rot)
{ fRotateXZ = rot*M_PI/180.; } // convert deg to rad
void STSpaceChargeTask::ShiftDisplacementMap(double x, double y, double z)
{ fOffsetX = x; fOffsetY = y; fOffsetZ = z; }

void STSpaceChargeTask::ExportDisplacementMap(const std::string& value)
{
  if(!fDispX || !fDispY || !fDispZ) 
    fLogger->Warning(MESSAGE_ORIGIN, "No displacement map is loaded. You need to either initialize this class or invoke CalculateEDrift method");
  TFile file(value.c_str(), "UPDATE");
  if(fDispX) fDispX->Write("shiftX_data");
  if(fDispY) fDispY->Write("shiftY_data");
  if(fDispZ) fDispZ->Write("shiftZ_data");
}

void STSpaceChargeTask::ExportEField(const std::string& value)
{
  TFile file(value.c_str(), "UPDATE");
  if(fEx) fEx->Write();
  if(fEy) fEy->Write();
  if(fEz) fEz->Write();
}

ClassImp(STSpaceChargeTask);
