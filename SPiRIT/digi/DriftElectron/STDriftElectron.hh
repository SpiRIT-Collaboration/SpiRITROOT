/**
 * @brief Calculate position of electron at the anode wire plane passed from 
 * STDriftTask.
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail This class recieves initial position of electron as a input and
 * returns position and time of electron at the anode wire plane as a output. 
 *
 * Position is corrected with diffusion parameters and time is calculated with drift
 * velocity. Drift velocity is constant but there is also difusion along
 * drifting axis. These parameters(diffusion and drift velocity) are calculated
 * from Garfield++. 
 */

#pragma once

#include "STDigiPar.hh"
#include "STGas.hh"
#include "TLorentzVector.h"

class STDriftElectron
{
  public :
    STDriftElectron();
    ~STDriftElectron();

    /**
     * MC hit creates not just one electron. So we first set MC initial hit,
     * calculate drift time, diffusion sigmas before calcualating 
     * diffusions for each electrons in Drift method.
     */
    void SetMCHit(TLorentzVector v4MC);
    /**
     * Proceed drifting task. This method randomize electron position and time 
     * stamp with calculated sigmas in SetMCHit method.
     */
    TLorentzVector Drift();

  private : 
    STDigiPar* fPar;      //!< Base parameter container
    STGas*     fGas;      //!< Gas parameter container

    Double_t fWirePlaneY; //!< y-position of wire plane [mm]
    Double_t fVelDrift;   //!< drift length [mm/ns]
    Double_t fCoefDL;     //!< longitudinal drift coefficient
    Double_t fCoefDT;     //!< transveral drift coefficient

    Double_t fDriftTime;  //!< Drift time of electron [ns]

    Double_t fSigmaDL;    //!< Diffusion sigma in longitudinal direction [mm]
    Double_t fSigmaDT;    //!< Diffusion sigma in transversal direction [mm]

    TLorentzVector fV4MC; //!< Poisition and time of MC data.

  ClassDef(STDriftElectron, 1);
};
