#ifndef STDRIFTELECTRON_HH
#define STDRIFTELECTRON_HH

#include "STDigiPar.hh"
#include "STGas.hh"
#include "TLorentzVector.h"

class STDriftElectron
{
  public :
    STDriftElectron(STDigiPar* par, STGas* gas);
    ~STDriftElectron() {};

    void SetMCHit(TLorentzVector positionTimeMC);
    TLorentzVector Drift();

  private : 
    STGas*     fGas;
    STDigiPar* fPar;

    Double_t fWirePlaneY; /// y-position of wire plane
    Double_t fVelDrift;   /// drift length
    Double_t fCoefDL;     /// longitudinal drift coefficient
    Double_t fCoefDT;     /// transveral   drift coefficient


    // SetMCHit()
    Double_t y;
    Double_t lDrift;
    Double_t tDrift;

    Double_t sigmaDL;
    Double_t sigmaDT;

    // Drift()
    Double_t dtDrift;
    Double_t dxz;
    Double_t dx;
    Double_t dz;
    Double_t angle;

    TLorentzVector fPositionTimeMC;

  ClassDef(STDriftElectron, 1);
};

#endif
