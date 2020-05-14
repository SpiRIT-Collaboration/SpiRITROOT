#ifndef STSPACECHARGE
#define STSPACECHARGE

// Fair class header
#include "FairLogger.h"
#include <string>
#include <functional>
// SPiRIT-TPC class headers
#include "STDigiPar.hh"
#include "FairField.h"

// ROOT class headers
#include "TClonesArray.h"
#include "TVector3.h"
#include "TH3.h"

/*************
 * helper classes for calculation of electron drift due to space charge
 *************/
class EquationOfMotion
{
public:
  typedef std::function<TVector3(const TVector3&)> FieldFunc;
  EquationOfMotion(const FieldFunc& t_EField,
                   const FieldFunc& t_BField,
                   double t_mu,
                   double t_wtau);
  TVector3 operator()(const TVector3& t_pos, double t_t);
private:
  FieldFunc EField_;
  FieldFunc BField_;
  double mu_, wtau_;
};

TVector3 RK4Stepper(EquationOfMotion& t_eom, const TVector3& t_pos, double t_t, double t_dt);

class ElectronDrifter
{
public:
  ElectronDrifter(double t_dt, EquationOfMotion& t_eom);
  ElectronDrifter& DriftUntil(const std::function<bool(const TVector3&, double)>&);
  TVector3 DriftFrom(const TVector3& t_pos);
  double GetDriftTime();

private:
  const double dt_;
  double t_;
  const double ymax_ = 0; // stopping condition
  std::function<bool(const TVector3&, double)> stop_cond_; // setoping condition
  EquationOfMotion eom_;
};

/**************************
* Begin of Space charge task
* *************************/
class STSpaceCharge
{
  public:
    enum class Projectile{Sn108, Sn112, Sn124, Sn132};

    STSpaceCharge(double width=864, double length=1344, double height=506.1);  //!< Default constructor
    // inputs: size of the TPC in mm
    ~STSpaceCharge(); //!< Destructor

    void SetTPCSize(double width, double length, double height);
    void SetXYZBins(int xbins, int ybins, int zbins);
    void SetBField(FairField* Bfield);
    void SetEFieldSolution(const std::string& value);
    void SetSheetChargeDensity(Double_t value, Double_t bf = 0);
    void SetProjectile(Projectile t_proj);
    void SetDriftParameters(double mu, double wtau);
    void InferDriftParameters(double drift_vel, double e_field, double b_field);

    void ShiftDisplacementMap(double x, double y, double z);
    void RotateEFieldXZDeg(double rot);
    void ExportDisplacementMap(const std::string& value);
    void ExportEField(const std::string& value);
    void CalculateEDrift(double drift_vel, bool t_invert=false);

    void DisplaceElectrons(double x, double y, double z, 
                           double& x_out, double& y_out, double& z_out);
  private:
    void fFinalizeEField();
    std::function<TVector3(const TVector3&)> GetEFieldWrapper();
    std::function<TVector3(const TVector3&)> GetBFieldWrapper();

    FairLogger *fSCLogger;
    FairField* fBField; //!< Samurai magnetic field map
    TH3D *fEx;//!< E-field after space charge
    TH3D *fEy;//!< E-field after space charge
    TH3D *fEz;//!< E-field after space charge

    TH3D *fDispX;//!< Calculated electron displacement map
    TH3D *fDispY;//!< Calculated electron displacement map
    TH3D *fDispZ;//!< Calculated electron displacement map
  
    double fWidth, fLength, fHeight;// physical size of the TPC.
    Double_t fSheetChargeDensity;
    Double_t fBackFlowDensity = 0;
    std::string fEFieldFile;
    Projectile fProj;
    const std::map<Projectile, std::string> fProjName;
  
    double fDt; // time step in electron drift calculation
    double fBinsX, fBinsY, fBinsZ; // number of bines to divide the displacement map
    double fOffsetX, fOffsetY, fOffsetZ;
    double fRotateXZ;
    double fmu, fwtau;
    STSpaceCharge(const STSpaceCharge&);
    STSpaceCharge operator=(const STSpaceCharge&);


  ClassDef(STSpaceCharge,1);
};

#endif
