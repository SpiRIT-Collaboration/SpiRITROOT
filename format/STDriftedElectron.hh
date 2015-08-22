/**
 * @brief Information of electrons produced in STDriftTask.
 *
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STDRIFTEDELECTRON
#define STDRIFTEDELECTRON

#include "TObject.h"

class STDriftedElectron : public TObject
{
  public :

    /** Default constructor **/
    STDriftedElectron();

    /** Constructor with input **/
    STDriftedElectron(Double_t x,
                      Double_t diffusedX,
                      Double_t z,
                      Double_t diffusedZ,
                      Double_t hitY,
                      Double_t hitTime,
                      Double_t driftTime,
                      Double_t diffusedTime,
                      Int_t    iWire,
                      Int_t    zWire,
                      Int_t    gain);

    /** Default destructor **/
    ~STDriftedElectron();

    Double_t GetX();
    Double_t GetDiffusedX();
    Double_t GetZ();
    Double_t GetDiffusedZ();
    Double_t GetHitY();
    Double_t GetHitTime();
    Double_t GetDriftTime();
    Double_t GetDiffusedTime();
    Int_t    GetIWire();
    Int_t    GetZWire();
    Int_t    GetGain();

    void SetIndex(Int_t index);
    
  private :
    Double_t fX;            ///< x-position (diffusion is not included) [mm]
    Double_t fDiffusedX;    ///< x-diffusion [mm]
    Double_t fZ;            ///< z-position (diffusion is not included) [mm]
    Double_t fDiffusedZ;    ///< z-diffusion [mm]
    Double_t fHitY;         ///< y-position of MC hit [mm]
    Double_t fHitTime;      ///< time of MC hit[ns]
    Double_t fDriftTime;    ///< drift time [ns]
    Double_t fDiffusedTime; ///< diffused time [ns]
    Int_t    fIWire;        ///< numbering of wire. (negative value is posible)
    Int_t    fZWire;        ///< z-position of wire where electron is absorbed [mm]
    Int_t    fGain;         ///< amount of gain in wire plane

    Int_t    fIndex;        ///< position of STDriftedElectron in "TClonesArray"

  ClassDef(STDriftedElectron, 2)
};

#endif
