/**
 * @brief Process drifting of electron from created position to anode wire
 * plane. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail This class recieves STMCPoint as a input and returns
 * STDriftedElectron as a output. 
 *
 * Calculate Number of electrons created from ionization of gas. 
 * Each electron is drifted along y-direction by electric field. Position in
 * the anode wire plane is calculated based on drift velocity and diffusion 
 * parameters. 
 *
 * Position is corrected with diffusion parameters and time is calculated with drift
 * velocity. Drift velocity is constant but there is also difusion along
 * drifting axis. These parameters(diffusion and drift velocity) are calculated
 * from Garfield++. 
 *
 * Once electron reaches anode wire pane, electron is absorbed in the closest wire. 
 * Thus z-component of STDriftedElectron is z-position of corresponding wire 
 */

#ifndef STSPACECHARGETASK
#define STSPACECHARGETASK

// Fair class header
#include "FairTask.h"
#include <string>
#include <functional>
// SPiRIT-TPC class headers
#include "STMCPoint.hh"
#include "STDriftedElectron.hh"
#include "STDigiPar.hh"
#include "STFieldMap.hh"

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
  TVector3 DriftFrom(const TVector3& t_pos);
  double GetDriftTime();

private:
  const double dt_;
  double t_;
  const double ymax_ = 0; // stopping condition
  EquationOfMotion eom_;
};

/**************************
* Begin of Space charge task
* *************************/
class STSpaceChargeTask : public FairTask
{
  public:

    STSpaceChargeTask();  //!< Default constructor
    ~STSpaceChargeTask(); //!< Destructor

    virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
    virtual void Exec(Option_t* opt); //!< Executed for each event.
    virtual void SetParContainers();  //!< Load the parameter container from the runtime database.

   void SetCustomMap(TH3D* dispx, TH3D* dispy, TH3D* dispz);
   void SetBField(STFieldMap* Bfield);
   void SetEFieldSolution(const std::string& value);
   void SetBeamRate(Double_t value);
   void SetProjectile(const std::string& value);
   void ExportDisplacementMap(const std::string& value);
   void ExportEField(const std::string& value);

   void SetPersistence(Bool_t value = kTRUE);
   void SetElectronDrift(Bool_t value = kTRUE);
   void SetVerbose(Bool_t value = kTRUE);
  
  private:
    void fSpaceChargeEffect(double x, double y, double z, 
                           double& x_out, double& y_out, double& z_out);

    void fSetEField();
    Double_t fBeamRate;
    std::string fEFieldFile;
    std::string fProj;

    Bool_t fIsPersistence;  ///< Persistence check variable
    Bool_t fVerbose; // testing with cout 
    Bool_t fIsDrift; // flag to make space charge effect optional
    Int_t fEventID; //!< EventID
    

    TClonesArray* fMCPointArray;     //!< [INPUT] Array of STMCPoint.
    STMCPoint* fMCPoint;             //!< [INPUT] MC data container (position, time, energyloss etc.)
    TClonesArray* fDispMCPointArray;    //!< [OUTPUT] Array of displaced STMCPoint due to E and B-field.
    STDigiPar* fPar; //!< Base parameter container

    STFieldMap* fBField; //!< Samurai magnetic field map
    TH3D *fEx;//!< E-field after space charge
    TH3D *fEy;//!< E-field after space charge
    TH3D *fEz;//!< E-field after space charge

    TH3D *fDispX;
    TH3D *fDispY;
    TH3D *fDispZ;
    STSpaceChargeTask(const STSpaceChargeTask&);
    STSpaceChargeTask operator=(const STSpaceChargeTask&);


  ClassDef(STSpaceChargeTask,1);
};

#endif
