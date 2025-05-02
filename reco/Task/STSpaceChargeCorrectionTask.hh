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

  bool SearchForRunPar(const std::string& filename, int run_num);   

  void SetLocalRate(Double_t scale, Int_t events, Int_t frequency);



  void UpdateEDrift();

private:
  Bool_t fVerbose; // testing with cout 
  Bool_t fIsDrift; // flag to make space charge effect optional

  STAuxHeader *fAuxHeader = nullptr;

  Bool_t fUseLocalRate{false}; // flag to use the timestamp to obtain charge density
  Int_t fRateEvents; // how many events to average over for the rate
  Int_t fEventFrequency; // how many events between E-field updates
  Double_t fDensityScale; // scaling factor for density. Density = fDensityScale * local_event_rate 

  std::vector<ULong_t> fTSbuffer; // buffer for storing previous timestamps
  Int_t fEvents{0}; // number of events so far

  double TPCx, TPCz; //!< widht and length of the TPC respectively (in mm)
  double TPCy = 506.1; //!< Default height of the TPC
  
  TClonesArray *fHitClusterArray = nullptr;

  STSpaceChargeCorrectionTask(const STSpaceChargeCorrectionTask&);
  STSpaceChargeCorrectionTask operator=(const STSpaceChargeCorrectionTask&);


  ClassDef(STSpaceChargeCorrectionTask,1);
};

#endif
