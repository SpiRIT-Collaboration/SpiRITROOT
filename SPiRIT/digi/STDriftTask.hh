/**
 * @brief Process drifting of electron from created position to anode wire
 * plane. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail This class recieves STMCPoint as a input and returns
 * STDriftedElectron as a output. 
 *
 * Calculate Number of electrons created from ionization of gas. 
 * Each electron is drifted along y-direction by electric field. Position in
 * the anode wire plane is calculated based on drift velocity and diffusion 
 * parameters(see STDriftElectron class for detail). Once electron reaches anode 
 * wire pane, electron is absorbed in the closest wire. Thus z-component of 
 * STDriftedElectron is z-position of corresponding wire(see STWireResponse 
 * class for detail). 
 */

#pragma once

// Fair class header
#include "FairTask.h"

// SPiRIT-TPC class headers
#include "STMCPoint.hh"
#include "STDriftedElectron.hh"
#include "STDriftElectron.hh"
#include "STWireResponse.hh"
#include "STDigiPar.hh"
#include "STGas.hh"

// ROOT class headers
#include "TClonesArray.h"

class STDriftTask : public FairTask
{
  public:

    STDriftTask();  //!< Default constructor
    ~STDriftTask(); //!< Destructor

    virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
    virtual void Exec(Option_t* opt); //!< Executed for each event.
    virtual void SetParContainers();  //!< Load the parameter container from the runtime database.

  private:

    STDriftElectron* fDriftElectron; //!< See STDriftElectron class for detail. 
    STWireResponse*  fWireResponse;  //!< See STWireResponse class for detail. 

    TClonesArray* fMCPointArray;     //!< [INPUT] Array of STMCPoint.
    STMCPoint* fMCPoint;             //!< [INPUT] MC data container (position, time, energyloss etc.)

    TClonesArray* fElectronArray;    //!< [OUTPUT] Array of STDriftedElectron.

    STDigiPar* fPar; //!< Base parameter container.
    STGas*     fGas; //!< Gas parameter container.

    Double_t fEIonize; //!< Ionization energy [eV]

    STDriftTask(const STDriftTask&);
    STDriftTask operator=(const STDriftTask&);


  ClassDef(STDriftTask,1);
};
