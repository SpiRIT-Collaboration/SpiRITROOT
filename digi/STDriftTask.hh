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
 * parameters. 
 *
 * Position is corrected with diffusion parameters and time is calculated with drift
 * velocity. Drift velocity is constant but there is also difusion along
 * drifting axis. These parameters(diffusion and drift velocity) are calculated
 * from Garfield++. 
 *
 * Once electron reaches anode wire pane, electron is absorbed in the closest wire. 
 * Thus z-component of STDriftedElectron is z-position of corresponding wire 
 */

#ifndef STDRIFTTASK
#define STDRIFTTASK

// Fair class header
#include "FairTask.h"

// SPiRIT-TPC class headers
#include "STMCPoint.hh"
#include "STDriftedElectron.hh"
#include "STDigiPar.hh"

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

    Int_t fEventID; //!< EventID

    TClonesArray* fMCPointArray;     //!< [INPUT] Array of STMCPoint.
    STMCPoint* fMCPoint;             //!< [INPUT] MC data container (position, time, energyloss etc.)

    TClonesArray* fElectronArray;    //!< [OUTPUT] Array of STDriftedElectron.

    STDigiPar* fPar; //!< Base parameter container.

    Double_t fYAnodeWirePlane; //!< y-position of anode wire plane. [mm]
    Double_t fZWidthPadPlane;  //!< z-width of pad plane.

    Int_t fNumWires;     //!< Total number of wires.
    Int_t fZCenterWire;  //!< z-position of center wire. [mm]
    Int_t fZSpacingWire; //!< z Spacing value of wires. [mm]
    Int_t fZOffsetWire;  //!< z Offset of wires. [mm]
    Int_t fZFirstWire;   //!< z-position of first wire. (minimum z) [mm]
    Int_t fZLastWire;    //!< z-position of last wire. (maximum z) [mm]
    Int_t fZCritWire;    //!< z-position of criterion wire. (minimum positive z) [mm]
    Int_t fIFirstWire;   //!< Numbering of first wire.
    Int_t fILastWire;    //!< Numbering of last wire.
    Int_t fICritWire;    //!< Numbering of criterion wire. (should be 0)

    Double_t fEIonize;  //!< Effective ionization energy of gas. [eV]
    Double_t fVelDrift; //!< Drift velocity of electron in gas. [mm/ns]
    Double_t fCoefT;    //!< Transversal diffusion coefficient. [mm^(-1/2)]
    Double_t fCoefL;    //!< Longitudinal diffusion coefficient. [mm^(-1/2)]
    Double_t fGain;     //!< Gain.

    STDriftTask(const STDriftTask&);
    STDriftTask operator=(const STDriftTask&);


  ClassDef(STDriftTask,1);
};

#endif
