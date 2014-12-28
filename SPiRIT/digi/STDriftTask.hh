//---------------------------------------------------------------------
// Description:
//      Drift electron task class headers
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#pragma once

// Fair class header
#include "FairTask.h"

// SPiRIT-TPC class headers
#include "STMCPoint.hh"
#include "STDigitizedElectron.hh"
#include "STDriftElectron.hh"
#include "STWireResponse.hh"
#include "STDigiPar.hh"
#include "STGas.hh"

// ROOT class headers
#include "TClonesArray.h"

class STDriftTask : public FairTask
{
  public:

    /** Default constructor **/
    STDriftTask();

    /** Constructor with parameters (Optional) **/
    //  STDriftTask(Int_t verbose);


    /** Destructor **/
    ~STDriftTask();


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

    /** Drift **/
    STDriftElectron* fDriftElectron;

    /** Wire **/
    STWireResponse*  fWireResponse;

    /** Input array from previous already existing data level **/
    TClonesArray* fMCPointArray;
    STMCPoint*    fMCPoint;

    /** Output array to  new data level**/
    TClonesArray* fDigitizedElectronArray;
    STDigitizedElectron* fDigiElectron;

    /** Parameter containers **/
    STDigiPar* fPar;
    STGas*     fGas;

    /** Parameters **/
    Double_t fEIonize;


    STDriftTask(const STDriftTask&);
    STDriftTask operator=(const STDriftTask&);



  ClassDef(STDriftTask,1);
};
