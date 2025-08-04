#ifndef STSPACECHARGETASK
#define STSPACECHARGETASK

// Fair class header
#include "FairTask.h"
#include "FairMCEventHeader.h"

#include <string>
#include <functional>
// SPiRIT-TPC class headers
#include "STAuxHeader.hh"
#include "STSpaceCharge.hh"
#include "STMCPoint.hh"
#include "STDriftedElectron.hh"
#include "STDigiPar.hh"
#include "FairField.h"

// ROOT class headers
#include "TClonesArray.h"
#include "TVector3.h"
#include "TH3.h"

/**************************
* Begin of Space charge task
* *************************/
class STSpaceChargeTask : public FairTask, public STSpaceCharge
{
  public:

    STSpaceChargeTask();  //!< Default constructor
    ~STSpaceChargeTask(); //!< Destructor

    virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
    virtual void Exec(Option_t* opt); //!< Executed for each event.
    virtual void SetParContainers();  //!< Load the parameter container from the runtime database.

    void SetLocalRate(Double_t scale_s, Double_t scale_i, TString filename, TString histname = "timeHist500", Int_t frequency = 500); //Use for 2024 charge density rate dependence and event dependent rate
    void SetLocalRate(Double_t scale_s, Double_t scale_i, Double_t run_rate); //Use for 2024 charge density rate dependence with a fixed rate for the run

    void UpdateEDrift();

   void SetPersistence(Bool_t value = kTRUE);
   void SetVerbose(Bool_t value = kTRUE);

   private:
    Bool_t fIsPersistence;  ///< Persistence check variable
    Bool_t fVerbose; // testing with cout 
    Int_t fEventID; //!< EventID
    
    TClonesArray* fMCPointArray;     //!< [INPUT] Array of STMCPoint.
    STMCPoint* fMCPoint;             //!< [INPUT] MC data container (position, time, energyloss etc.)
    STDigiPar* fPar; //!< Base parameter container
    FairMCEventHeader *fFairMCEventHeader; //!<

    STAuxHeader *fAuxHeader = nullptr;

    Bool_t fUseLocalRate{false}; // flag to use the timestamp to obtain charge density
    Int_t fEventFrequency; // how many events between E-field updates
    Double_t fDensityScaleSlope; // scaling factor slope for density. Density = (slope * rate + inter) * rate
    Double_t fDensityScaleInter; // scaling factor interceptfor density. Density = (slope * rate + inter) * rate
    Bool_t fFirstEventDone{false}; // flag to see if the rate was set for the first event when using the local rate

    TH1D *fRateHist; //histogram for the rate.

    STSpaceChargeTask(const STSpaceChargeTask&);
    STSpaceChargeTask operator=(const STSpaceChargeTask&);


  ClassDef(STSpaceChargeTask,1);
};

#endif
