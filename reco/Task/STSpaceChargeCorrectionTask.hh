#ifndef STSPACECHARGECORRECTIONTASK
#define STSPACECHARGECORRECTIONTASK

// Fair class header
#include "FairTask.h"
#include "FairLogger.h"
#include <string>
// SPiRIT-TPC class headers
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

private:
  Bool_t fVerbose; // testing with cout 
  Bool_t fIsDrift; // flag to make space charge effect optional

  double TPCx, TPCz; //!< widht and length of the TPC respectively (in mm)
  double TPCy = 506.1; //!< Default height of the TPC
  
  TClonesArray *fHitClusterArray = nullptr;

  STSpaceChargeCorrectionTask(const STSpaceChargeCorrectionTask&);
  STSpaceChargeCorrectionTask operator=(const STSpaceChargeCorrectionTask&);


  ClassDef(STSpaceChargeCorrectionTask,1);
};

#endif
