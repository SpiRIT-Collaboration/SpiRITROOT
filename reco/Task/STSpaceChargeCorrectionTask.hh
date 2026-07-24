#ifndef STSPACECHARGECORRECTIONTASK
#define STSPACECHARGECORRECTIONTASK

// Fair class header
#include "FairTask.h"
#include "FairLogger.h"
#include <string>
// SPiRIT-TPC class headers
#include "STAuxHeader.hh"
#include "STSpaceCharge.hh"
#include "STDigiPar.hh"
#include "STRecoTask.hh"
#include "STHitCluster.hh"

// ROOT class headers
#include "TClonesArray.h"
#include "TVector3.h"

class STSpaceChargeCorrectionTask : public STSpaceCharge, public STRecoTask
{
public:
  STSpaceChargeCorrectionTask(); //!< Default constructor
  ~STSpaceChargeCorrectionTask(); //!< Destructor

  virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
  virtual void Exec(Option_t* opt); //!< Executed for each event.

  void SetVerbose(Bool_t value = kTRUE);
  void SetElectronDrift(Bool_t value = kTRUE);
  void SetDriftVelocity(Double_t value);

  bool SearchForRunPar(const std::string& filename, int run_num);   

  void SetLocalRate(Double_t scale_s, Double_t scale_i, TString filename, TString histname = "timeHist500", Int_t frequency = 500);

  void UpdateEDrift();

private:
  Bool_t fVerbose; // testing with cout 
  Bool_t fIsDrift; // flag to make space charge effect optional
  Bool_t fUseExternalDriftVelocity{kFALSE};
  Double_t fDriftVelocity{0.};

  STAuxHeader *fAuxHeader = nullptr;

  Bool_t fUseLocalRate{false}; // flag to use the timestamp to obtain charge density
  Int_t fEventFrequency; // how many events between E-field updates
  Double_t fDensityScaleSlope; // scaling factor slope for density. Density = (slope * rate + inter) * rate
  Double_t fDensityScaleInter; // scaling factor interceptfor density. Density = (slope * rate + inter) * rate
  Bool_t fFirstEventDone{false}; // flag to see if the rate was set for the first event when using the local rate

  TH1D *fRateHist; //histogram for the rate.


  double TPCx, TPCz; //!< widht and length of the TPC respectively (in mm)
  double TPCy = 506.1; //!< Default height of the TPC
  
  TClonesArray *fHitClusterArray = nullptr;

  STSpaceChargeCorrectionTask(const STSpaceChargeCorrectionTask&);
  STSpaceChargeCorrectionTask operator=(const STSpaceChargeCorrectionTask&);


  ClassDef(STSpaceChargeCorrectionTask,1);
};

#endif
