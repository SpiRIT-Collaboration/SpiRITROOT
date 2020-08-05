#ifndef STKYOTOTASK
#define STKYOTOTASK

// Fair class header
#include "FairTask.h"
#include "FairMCEventHeader.h"

#include <string>
#include <functional>
// SPiRIT-TPC class headers
#include "STMCPoint.hh"
#include "STDigiPar.hh"
#include "FairField.h"

// ROOT class headers
#include "TClonesArray.h"

/**************************
* Begin of Space charge task
* *************************/
class STKyotoTask : public FairTask
{
  public:

    STKyotoTask();  //!< Default constructor
    ~STKyotoTask(); //!< Destructor

    virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
    virtual void Exec(Option_t* opt); //!< Executed for each event.
    virtual void SetParContainers();  //!< Load the parameter container from the runtime database.

   void SetVerbose(Bool_t value = kTRUE);

   private:
    Bool_t fVerbose; // testing with cout 
    Int_t fEventID; //!< EventID
    
    TClonesArray* fMCPointArray;     //!< [INPUT] Array of STMCPoint.
    STMCPoint* fMCPoint;             //!< [INPUT] MC data container (position, time, energyloss etc.)
    STDigiPar* fPar; //!< Base parameter container
    FairMCEventHeader *fFairMCEventHeader; //!<
    Double_t fEnclosureWidth;

  ClassDef(STKyotoTask,1);
};

#endif
