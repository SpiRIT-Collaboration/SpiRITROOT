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
#include "STEventHeader.hh"

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
   void UseKatana(Bool_t value = kTRUE) { fUseKatana = value; }
   private:
    Bool_t fVerbose; // testing with cout 
    Int_t fEventID; //!< EventID
    
    TClonesArray* fMCPointArray;     //!< [INPUT] Array of STMCPoint.
    STMCPoint* fMCPoint;             //!< [INPUT] MC data container (position, time, energyloss etc.)
    STDigiPar* fPar; //!< Base parameter container
    FairMCEventHeader *fFairMCEventHeader; //!<
    STEventHeader *fEventHeader; //!
    Double_t fEnclosureWidth;
    Bool_t fUseKatana = false; //!

  ClassDef(STKyotoTask,1);
};

#endif
