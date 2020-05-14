#include "STSpaceCharge.hh"

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
    pos = RK4Stepper(eom_, pos, t_, dt_);
    t_ += dt_;
  }
  return pos;
};

double ElectronDrifter::GetDriftTime() { return t_; };


STSpaceCharge::STSpaceCharge(double width, double length, double height)
:fSCLogger(nullptr),
 fWidth(width/10),
 fLength(length/10),
 fHeight(height/10),
 fBinsX(35), 
 fBinsY(30),
 fBinsZ(50),
 fBField(nullptr),
 fDispX(nullptr),
 fDispY(nullptr),
 fDispZ(nullptr),
 fOffsetX(0),
 fOffsetY(0),
 fOffsetZ(0),
 fRotateXZ(0),
 fmu(-4.3556e4),
 fwtau(-2.18),
 fProj(Projectile::Sn132),
 fProjName({{Projectile::Sn108, "108Sn"}, 
            {Projectile::Sn112, "112Sn"}, 
            {Projectile::Sn124, "124Sn"}, 
            {Projectile::Sn132, "132Sn"}})
{
  fSCLogger = FairLogger::GetLogger();
  fSCLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STSpaceCharge");

  // Default value
  std::string vmc_dir = std::getenv("VMCWORKDIR");
  fEFieldFile = vmc_dir + "/input/E-field.root";
  fSheetChargeDensity = 3.14e-8;
}

STSpaceCharge::~STSpaceCharge()
{
  fSCLogger->Debug(MESSAGE_ORIGIN,"Destructor of STSpaceCharge");
  if(fEx) fEx->Delete();
  if(fEy) fEy->Delete();
  if(fEz) fEz->Delete();
}

// input: mm, need to convert to cm internally
void STSpaceCharge::SetTPCSize(double width, double length, double height)
{ fWidth = width/10; fLength = length/10; fHeight = height/10; }

void STSpaceCharge::SetXYZBins(int xbins, int ybins, int zbins)
{ fBinsX = xbins; fBinsY = ybins; fBinsZ = zbins; }

void STSpaceCharge::fFinalizeEField()
{
  const std::string Proj = fProjName.at(fProj);
  fSCLogger->Info(MESSAGE_ORIGIN,("Loading E-field solution from " + fEFieldFile + " for projectile " + Proj).c_str());
  TFile file(fEFieldFile.c_str());
  if(!file.IsOpen()) fSCLogger->Fatal(MESSAGE_ORIGIN, "E-field file cannot be opened!");
  auto homox = static_cast<TH3D*>(file.Get(("homo_" + Proj + "_Ex").c_str()));
  auto homoy = static_cast<TH3D*>(file.Get(("homo_" + Proj + "_Ey").c_str()));
  auto homoz = static_cast<TH3D*>(file.Get(("homo_" + Proj + "_Ez").c_str()));

  auto nohomox = static_cast<TH3D*>(file.Get(("nohomo_" + Proj + "_Ex").c_str()));
  auto nohomoy = static_cast<TH3D*>(file.Get(("nohomo_" + Proj + "_Ey").c_str()));
  auto nohomoz = static_cast<TH3D*>(file.Get(("nohomo_" + Proj + "_Ez").c_str()));

  auto bfx = static_cast<TH3D*>(file.Get(("bf_" + Proj + "_Ex").c_str()));
  auto bfy = static_cast<TH3D*>(file.Get(("bf_" + Proj + "_Ey").c_str()));
  auto bfz = static_cast<TH3D*>(file.Get(("bf_" + Proj + "_Ez").c_str()));

  fSCLogger->Info(MESSAGE_ORIGIN,TString::Format("E-field is calculated with beam rate = %e and backflow density = %e", fSheetChargeDensity, fBackFlowDensity));
  double factor = fSheetChargeDensity / 3.14e-8; //3.14e-8 is the beam rate of run 2841
  double bfFactor = fBackFlowDensity / 3.14e-8;
  if(homox && homoy && homoz && nohomox && nohomoy && nohomoz)
  {
    homox->Add((nohomox->Scale(factor), nohomox));
    homoy->Add((nohomoy->Scale(factor), nohomoy));
    homoz->Add((nohomoz->Scale(factor), nohomoz));

    if(bfx && bfy && bfz)
    {
      homox->Add((bfx->Scale(bfFactor), bfx));
      homoy->Add((bfy->Scale(bfFactor), bfy));
      homoz->Add((bfz->Scale(bfFactor), bfz));
    }
    else fSCLogger->Info(MESSAGE_ORIGIN, "No backflow E-field is found. It's contribution to E-field will be ignored");

    fEx = static_cast<TH3D*>(homox->Clone("shiftX"));
    fEy = static_cast<TH3D*>(homoy->Clone("shiftY"));
    fEz = static_cast<TH3D*>(homoz->Clone("shiftZ"));
    fEx -> SetDirectory(0); // disable memory management for this histogram
    fEy -> SetDirectory(0);
    fEz -> SetDirectory(0);
  }
  else fSCLogger->Fatal(MESSAGE_ORIGIN, "E-field cannot be loaded from files.");
}

// drift_vel in cm/um, e_field in V/cm
void STSpaceCharge::InferDriftParameters(double drift_vel, double e_field, double b_field)
{
  fSCLogger->Info(MESSAGE_ORIGIN, "Infering electron drift parameters from drift velocity");
  const double me = 9.11e-31; // mass of electrons in kg
  const double echarge = 1.6e-19; // charge of electrons 

  e_field *= 100; // V/cm to V/m
  drift_vel *= 10000; // cm/um to m/s
  double tau = drift_vel*me/(echarge*e_field); // in s
   
  double omega = echarge*b_field/me; // rad/s
  double wtau = omega*tau;
  double mu = drift_vel/e_field*10000;

  // both mu and wtau are signed
  this->SetDriftParameters(-mu, -wtau);
}

void STSpaceCharge::CalculateEDrift(double drift_vel, bool t_invert)
{
  if(!fBField) fSCLogger->Fatal(MESSAGE_ORIGIN, "B-field is not loaded");
  else fBField->Init();
  gErrorIgnoreLevel = kFatal;
  fSCLogger->Info(MESSAGE_ORIGIN, "Begin calculation of electron displacement map");
  this -> fFinalizeEField();

  // Rewrite E and B field in a form that can be read by ElectronDrifter
  auto EFieldWrapper = this->GetEFieldWrapper();
  auto BFieldWrapper = this->GetBFieldWrapper();
  auto eom = EquationOfMotion(EFieldWrapper, BFieldWrapper, fmu, fwtau); 
  const double dt = (t_invert)? -5e-8:5e-8; // seconds
  auto drifter = ElectronDrifter(dt, eom);

  // mu= -4.252e4, wtau= -4 was found to reproduce E cross B result
  // still subjected to change, not finalized
  if(t_invert) fSCLogger->Info(MESSAGE_ORIGIN, "Dispacemant is inverted, i.e. arguments of the displacement map is the detected locattion");
  else fSCLogger->Info(MESSAGE_ORIGIN, "Displacement is not inverted, i.e. arguments of the displacement map is the physical location");
  fSCLogger->Info(MESSAGE_ORIGIN, TString::Format("Drift parameter: mu = %f, wtau = %f", fmu, fwtau));
  fSCLogger->Info(MESSAGE_ORIGIN, TString::Format("Displacement map is shifted by: x = %f, y = %f, z = %f", fOffsetX, fOffsetY, fOffsetZ));
  fSCLogger->Info(MESSAGE_ORIGIN, TString::Format("E field is rotated in the XZ plane by %f deg", fRotateXZ*180/M_PI));
  // create displacement map with the following dimensions:
  // all dimensions are in cm

  double x_min = -fWidth/2., x_max = fWidth/2.;
  double y_min = -fHeight, y_max = 0;
  double z_min = 0, z_max = fLength;

  fDispX = new TH3D("shiftX_data","shiftX",fBinsX,x_min,x_max,fBinsY,y_min,y_max,fBinsZ,z_min,z_max);
  fDispY = new TH3D("shiftY_data","shiftY",fBinsX,x_min,x_max,fBinsY,y_min,y_max,fBinsZ,z_min,z_max);
  fDispZ = new TH3D("shiftZ_data","shiftZ",fBinsX,x_min,x_max,fBinsY,y_min,y_max,fBinsZ,z_min,z_max);
   

  int counter = 1;
  int percentage = 0;
  std::cout << "STSpaceCharge Displacement map Calculation Percent completion:        ";
  for(int idx = 1; idx <= fBinsX; ++idx)
   for(int idy = 1; idy <= fBinsY; ++idy)
     for(int idz = 1; idz <= fBinsZ; ++idz)
     {
       double x = fDispX->GetXaxis()->GetBinCenter(idx);
       double y = fDispX->GetYaxis()->GetBinCenter(idy);
       double z = fDispX->GetZaxis()->GetBinCenter(idz);

       // initial position
       double diffx;
       double diffy=0;
       double diffz;
      
       TVector3 pos;
       if(t_invert)
       {
         pos[0] = x; pos[1] = 0; pos[2] = z;
         pos = drifter.DriftUntil([drift_vel, y](const TVector3& t_pos, double t){ return t*1e6*drift_vel < y; })
                      .DriftFrom(pos);
         //diffy = pos.Y()-y;
       }else
       {
         pos[0] = x; pos[1] = y; pos[2] = z;
         pos = drifter.DriftFrom(pos);
         //diffy = -drift_vel*drifter.GetDriftTime()*1e6-y;
       }
       diffx = pos.X()-x;
       diffz = pos.Z()-z ;
       
       fDispX->SetBinContent(idx, idy, idz, diffx);
       fDispY->SetBinContent(idx, idy, idz, diffy);// still not sure about how drift velocity is calculated. Recommend that we disable y drift for now (-drift_vel*drifter.GetDriftTime()*1e6-y));
       fDispZ->SetBinContent(idx, idy, idz, diffz);
       int new_progress =  int(counter/(double) (fBinsX*fBinsY*fBinsZ)*100);
       if(new_progress != percentage)
       {
         percentage = new_progress;
         std::cout << "\b\b\b\b\b" << std::setw(3) << percentage << " %" << std::flush;
       }
       ++counter;
     }
  std::cout << std::endl;
}

void STSpaceCharge::SetBField(FairField* Bfield){ fBField = Bfield; }
void STSpaceCharge::SetEFieldSolution(const std::string& value)
{
  std::string vmc_dir(std::getenv("VMCWORKDIR")); 
  fEFieldFile = vmc_dir + "/input/" + value; 
}

void STSpaceCharge::SetSheetChargeDensity(Double_t value, Double_t bf){ fSheetChargeDensity = value; fBackFlowDensity = bf; }
void STSpaceCharge::SetProjectile(Projectile value){ fProj = value; }
void STSpaceCharge::SetDriftParameters(double mu, double wtau) { fmu = mu; fwtau = wtau; }

std::function<TVector3(const TVector3&)> STSpaceCharge::GetEFieldWrapper()
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
      if(ey >= -10) ey = -124.7;
      return TVector3(rot_ex, ey, rot_ez);
    };
}

std::function<TVector3(const TVector3&)> STSpaceCharge::GetBFieldWrapper()
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


void STSpaceCharge::RotateEFieldXZDeg(double rot)
{ fRotateXZ = rot*M_PI/180.; } // convert deg to rad
void STSpaceCharge::ShiftDisplacementMap(double x, double y, double z)
{ fOffsetX = x; fOffsetY = y; fOffsetZ = z; }

void STSpaceCharge::ExportDisplacementMap(const std::string& value)
{
  if(!fDispX || !fDispY || !fDispZ) 
    fSCLogger->Warning(MESSAGE_ORIGIN, "No displacement map is loaded. You need to either initialize this class or invoke CalculateEDrift method");
  TFile file(value.c_str(), "RECREATE");
  if(fDispX) fDispX->Write("shiftX_data");
  if(fDispY) fDispY->Write("shiftY_data");
  if(fDispZ) fDispZ->Write("shiftZ_data");
}

void STSpaceCharge::ExportEField(const std::string& value)
{
  TFile file(value.c_str(), "RECREATE");
  if(fEx) fEx->Write();
  if(fEy) fEy->Write();
  if(fEz) fEz->Write();
}

void STSpaceCharge::DisplaceElectrons(double x, double y, double z, 
                                      double& x_out, double& y_out, double& z_out)
{
  // remember the unit is now in cm
  x_out = x + fDispX->Interpolate(x + fOffsetX, y + fOffsetY, z + fOffsetZ);
  y_out = y + fDispY->Interpolate(x + fOffsetX, y + fOffsetY, z + fOffsetZ); 
  z_out = z + fDispZ->Interpolate(x + fOffsetX, y + fOffsetY, z + fOffsetZ);
}


ClassImp(STSpaceCharge);
