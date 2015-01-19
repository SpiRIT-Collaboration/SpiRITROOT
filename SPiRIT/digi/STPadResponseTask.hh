/**
 * @brief Calculate pad response between anode wire and cathode pad. Put
 * charge data into pads(STPad).
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail This class receives STDriftedElectron as a input and returns
 * STRawEvent as a output.
 *
 * Calculate corresponding pads and time bucket for signal in wire plane. 
 * Pad response function is calculated from Gatti distribution. This function
 * gives signal distribution in function of position. Pads are filled base on
 * this function.
 *
 * Pad with no signal is neglected from STRawEvent.
 */

#pragma once 

// Fair class header
#include "FairTask.h"

// SPiRIT-TPC class headers
#include "STDriftedElectron.hh"
#include "STPadResponse.hh"
#include "STDigiPar.hh"
#include "STGas.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

// ROOT class headers
#include "TClonesArray.h"

class STPadResponseTask : public FairTask
{
  public:

    STPadResponseTask();  //!< Default constructor
    ~STPadResponseTask(); //!< Destructor

    virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
    virtual void Exec(Option_t* opt); //!< Executed for each event.
    virtual void SetParContainers();  //!< Load the parameter container from the runtime database.

  private:

    STPadResponse* fPadResponse;   //!< See STPadResponse class for detail.

    TClonesArray* fElectronArray;  //!< [INPUT] Array of STDriftedElectron.
    STDriftedElectron* fElectron;  //!< [INPUT] Electron data in anode wire plane.

    TClonesArray *fRawEventArray;  //!< [OUTPUT] Array of STRawEvent.
    STRawEvent* fRawEvent;         //!< [OUTPUT] Event data with array of STPad.

    STDigiPar* fPar; //!< Base parameter container.

    Int_t fNTBs; //!< Number of time buckets.



    STPadResponseTask(const STPadResponseTask&);
    STPadResponseTask operator=(const STPadResponseTask&);

  ClassDef(STPadResponseTask,1);
};
