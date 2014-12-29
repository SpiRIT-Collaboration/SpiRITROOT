#ifndef STELECTRONICSTASK_H
#define STELECTRONICSTASK_H

#include "FairTask.h"

// SPiRIT-TPC class headers
#include "STDigiPar.hh"
#include "STMap.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

// ROOT class headers
#include "TClonesArray.h"

class TClonesArray;

class STElectronicsTask : public FairTask
{
  public:

    /** Default constructor **/
    STElectronicsTask();

    /** Constructor with parameters (Optional) **/
    //  STElectronicsTask(Int_t verbose);


    /** Destructor **/
    ~STElectronicsTask();


    /** Initiliazation of task at the beginning of a run **/
    virtual InitStatus Init();

    /** ReInitiliazation of task when the runID changes **/
    virtual InitStatus ReInit();


    /** Executed for each event. **/
    virtual void Exec(Option_t* opt);

    /** Load the parameter container from the runtime database **/
    virtual void SetParContainers();

    /** Finish task called at the end of the run **/
    virtual void Finish();

  private:


    /** In/Output array to  new data level**/
    TClonesArray *fPPEventArray;
    TClonesArray *fRawEventArray;
    STRawEvent* fPPEvent;
    STRawEvent* fRawEvent;

    /** Parameter Container **/
    STDigiPar* fPar;

    /** Parameters **/
    Int_t fNTBs;
    Int_t fNBinPulser;
    Double_t fPulser[100];

    STElectronicsTask(const STElectronicsTask&);
    STElectronicsTask operator=(const STElectronicsTask&);

    ClassDef(STElectronicsTask,1);
};

#endif
