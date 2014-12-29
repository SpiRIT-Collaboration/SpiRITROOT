//---------------------------------------------------------------------
// Description:
//      Pad response task class header
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#pragma once 

// Fair class header
#include "FairTask.h"

// SPiRIT-TPC class headers
#include "STDigitizedElectron.hh"
#include "STWireResponse.hh"
#include "STPadResponse.hh"
#include "STDigiPar.hh"
#include "STGas.hh"
#include "STMap.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

// ROOT class headers
#include "TClonesArray.h"

class STPadResponseTask : public FairTask
{
  public:

    /** Default constructor **/
    STPadResponseTask();

    /** Constructor with parameters (Optional) **/
    //  STPadResponseTask(Int_t verbose);


    /** Destructor **/
    ~STPadResponseTask();


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
    void InitializeRawEvent();

    /** Wire & Pad **/
    STWireResponse* fWireResponse;
    STPadResponse*  fPadResponse;

    /** Input array from previous already existing data level **/
    TClonesArray* fDigitizedElectronArray;
    STDigitizedElectron* fDigiElectron;

    /** Output array to  new data level**/
    TClonesArray *fRawEventArray;
    STRawEvent* fRawEvent;

    /** Parameter Container **/
    STDigiPar* fPar;

    /** Pad Map **/
    STMap* fMap;

    /** Parameters **/
    Int_t fNTBs;



    STPadResponseTask(const STPadResponseTask&);
    STPadResponseTask operator=(const STPadResponseTask&);

  ClassDef(STPadResponseTask,1);
};
