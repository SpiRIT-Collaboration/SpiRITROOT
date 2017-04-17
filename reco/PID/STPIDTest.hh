#ifndef STPIDTEST_HH
#define STPIDTEST_HH

#include "STPID.hh"
#include "STRecoTask.hh"
#include "STTrackCandidate.hh"
#include "TObject.h"
#include "TF1.h"

/**
 * Units in [ADC] and [mm]
*/

class STPIDTest : public TObject
{
  public:
    STPIDTest();
    virtual ~STPIDTest() {}

    Double_t GetProbability(STPID::PID stpid, Double_t p, Double_t dedx);

    TF1 *GetdEdxFunction(STPID::PID stpid);
    void SetCoef(STPID::PID stpid, Double_t coef);

    Double_t MeVToADC(Double_t val);
    Double_t ADCToMeV(Double_t val);

    void SetADCToMeVFactor(Double_t val);
    void SetADCToMeVOffSet(Double_t val);
    Double_t GetADCToMeVFactor();
    Double_t GetADCToMeVOffSet();

  private:
    Double_t FitdEdxPion(Double_t *x, Double_t *par); ///< dEdx maximum in function of momentum (pion)
    Double_t FitdEdxProton(Double_t *x, Double_t *par); ///< dEdx maximum in function of momentum (proton)
    Double_t FitdEdxDeuteron(Double_t *x, Double_t *par); ///< dEdx maximum in function of momentum (deuteron)
    Double_t FitdEdxTriton(Double_t *x, Double_t *par); ///< dEdx maximum in function of momentum (triton)
    Double_t FitdEdx3He(Double_t *x, Double_t *par); ///< dEdx maximum in function of momentum (3He)
    Double_t FitdEdx4He(Double_t *x, Double_t *par); ///< dEdx maximum in function of momentum (4He)

  private:
    TF1 *fdEdxFunction[NUMSTPID];
    Double_t fCoef[NUMSTPID];

    TF1 *fGausFunction; ///< Gaussian used to calculate probability

    Double_t fADCToMeVFactor = 5.353e-06;
    Double_t fADCToMeVOffset = 0;

    ClassDef(STPIDTest, 1)
};

#endif
