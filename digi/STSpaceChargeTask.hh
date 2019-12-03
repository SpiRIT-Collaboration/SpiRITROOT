#ifndef STSPACECHARGETASK
#define STSPACECHARGETASK

// Fair class header
#include "FairTask.h"
#include "FairMCEventHeader.h"

#include <string>
#include <functional>
// SPiRIT-TPC class headers
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

    STSpaceChargeTask(const STSpaceChargeTask&);
    STSpaceChargeTask operator=(const STSpaceChargeTask&);


  ClassDef(STSpaceChargeTask,1);
};

#endif
