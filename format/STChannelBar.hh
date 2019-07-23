#ifndef STCHANNELBAR_HH
#define STCHANNELBAR_HH

#include "TObject.h"
#include "TVector3.h"
#include "TArrayD.h"
#include "TH1D.h"
#include "TF1.h"

#include <cmath>
using namespace std;

#include "STNeuLAND.hh"

class STChannelBar : public TObject
{
  private:
    Int_t fChannelID = -1;
    Int_t fLayer = -1;
    Int_t fRow = -1;

    Bool_t fIsAlongXNotY = true;
    TVector3 fBarCenter = TVector3(-999,-999,-999);
    Int_t fNumTDCBins = 1;

    Double_t fBarLength = -1; ///< [mm]
    Double_t fAttenuationLength = DBL_MAX; ///< [mm]
    Double_t fDecayTime = 1; ///< decay time of the pulse  [ns]
    Double_t fRiseTime = 0.1; ///< rise time of the pulse  [ns]
    Double_t fTimeErrorSigma = 0.1; ///< time resolution error sigma (of gaussian) [ns]
    Double_t fEffc = 140.; ///< [mm/ns]

    Int_t fPulseTDCRange = 0;

    Bool_t fFlagFindHit = false; //!
    Double_t fThreshold = 0;

    Double_t fADC[2];
    Double_t fQDC[2];
    Double_t fTDC[2];

    TH1D *fChannelL = nullptr;  //-> < Beam(+z) left(-x) or Beam below(-y)
    TH1D *fChannelR = nullptr;  //-> < Beam(+z) right(+x) or Beam above(+y)

    TF1  *fFuncPulse = nullptr; //!
    TH1D *fHistPulse = nullptr; //! < for fast calculation

    TH1D *fHistSpace = nullptr; //! < for Draw

  public:
    STChannelBar();

    STChannelBar(
           TString name,
           Int_t id,
           Int_t layer,
           Int_t row,
          Bool_t xy,
        TVector3 center,
           Int_t nbins = 100, // number of time bins
        Double_t barl = 2500, // [mm]
        Double_t attl = 1250, // [mm]
        Double_t dcyt = 2.1,  // [ns]
        Double_t rist = .01,  // [ns]
        Double_t terr = 0.15, // [ns]
        Double_t effc = 140.  // [mm/ns]
        );

    virtual ~STChannelBar() {
      //if (fChannelL !=nullptr) delete fChannelL;
      //if (fChannelR !=nullptr) delete fChannelR;
      //if (fHistSpace!=nullptr) delete fHistSpace;
      //if (fFuncPulse!=nullptr) delete fFuncPulse;
      //if (fHistPulse!=nullptr) delete fHistPulse;
    };

    void SetBar(
           TString name,
           Int_t id,
           Int_t layer,
           Int_t row,
          Bool_t xy,
        TVector3 center,
           Int_t nbins = 100, // number of time bins
        Double_t barl = 2500, // [mm]
        Double_t attl = 1250, // [mm]
        Double_t dcyt = 2.1,  // [ns]
        Double_t rist = .01,  // [ns]
        Double_t terr = 0.15, // [ns]
        Double_t effc = 140.  // [mm/ns]
        );

    void Init(TString name = "");

    virtual void Clear(Option_t *option="");
    virtual void Print(Option_t *option="") const;
    virtual void Draw (Option_t *option="");

    void Fill(TVector3 pos, Double_t adc);
    TVector3 FindHit(Double_t threshold, Bool_t forceFindHit = false);

    Int_t GetChannelID() const { return fChannelID; }
    Int_t GetLayer() const { return fLayer; }
    Int_t GetRow() const { return fRow; }

    Int_t GetIsALongXNotY() const { return fIsAlongXNotY; }
    TVector3 GetBarCenter() const { return fBarCenter; }
    Double_t GetBarCenterL() const { return (fIsAlongXNotY ? fBarCenter.X() : fBarCenter.Y()); }
    Int_t GetNumTDCBins() const { return fNumTDCBins; }

    Double_t GetBarLength() const { return fBarLength; }
    Double_t GetAttenuationLength() const { return fAttenuationLength; }
    Double_t GetDecayTime() const { return fDecayTime; }
    Double_t GetRiseTime() const { return fRiseTime; }
    Double_t GetTimeErrorSigma() const { return fTimeErrorSigma; }
    Double_t GetEffc() const { return fEffc; }

    Int_t GetPulseTDCRange() const { return fPulseTDCRange; }

    Double_t ConvertTDCToTime(Double_t tdc) const { return tdc / fNumTDCBins * fBarLength / fEffc; }
    Double_t ConvertTimeToTDC(Double_t time) const { return time * fEffc / fBarLength * fNumTDCBins; }

    Double_t ConvertLengthToTime(Double_t length) const { return length / fEffc; }
    Double_t ConvertLengthToTDC (Double_t length) const { return length / fBarLength * fNumTDCBins; }

    /// from local position
    Double_t ConvertPosToTime(Double_t pos) const { return (pos+fBarLength/2.) / fEffc; }
    Double_t ConvertPosToTDC (Double_t pos) const { return (pos+fBarLength/2.) / fBarLength * fNumTDCBins; }

    Double_t ConvertTDCToPos(Double_t tdc) const { return (tdc/fNumTDCBins - .5)*fBarLength + GetBarCenterL(); }

    TVector3 GetTDCHitPosition() const {
      auto pos = fBarCenter;
      auto x = ConvertTDCToPos(GetTDCByT());
      if (fIsAlongXNotY) pos.SetX(x);
      else pos.SetY(x);
      return pos;
    }

    TVector3 GetADCHitPosition() const {
      auto pos = fBarCenter;
      auto x = ConvertTDCToPos(GetTDCByA());
      if (fIsAlongXNotY) pos.SetX(x);
      else pos.SetY(x);
      return pos;
    }

    TVector3 GetQDCHitPosition() const {
      auto pos = fBarCenter;
      auto x = ConvertTDCToPos(GetTDCByQ());
      if (fIsAlongXNotY) pos.SetX(x);
      else pos.SetY(x);
      return pos;
    }

    Double_t GetChargeA()  const { return sqrt(fADC[0] * fADC[1] * exp(fBarLength/fAttenuationLength)); }
    Double_t GetChargeQ()  const { return sqrt(fQDC[0] * fQDC[1] * exp(fBarLength/fAttenuationLength)); }

    Double_t GetPosByA() const { return .5*(- fAttenuationLength * log(fADC[0]/fADC[1]) ); }
    Double_t GetPosByQ() const { return .5*(- fAttenuationLength * log(fQDC[0]/fQDC[1]) ); }

    Double_t GetTDCByA() const { return ConvertPosToTDC(GetPosByA()); }
    Double_t GetTDCByQ() const { return ConvertPosToTDC(GetPosByQ()); }
    Double_t GetTDCByT() const { return ( fTDC[0] + (fNumTDCBins - fTDC[1]) ) / 2.; }

    Double_t GetThreshold() const { return fThreshold; }

    Double_t GetADC(Int_t i) const { return fADC[i]; }
    Double_t GetQDC(Int_t i) const { return fQDC[i]; }
    Double_t GetTDC(Int_t i) const { return fTDC[i]; }

    TArrayD *GetChannelL() { return fChannelL; } 
    TArrayD *GetChannelR() { return fChannelR; } 

    TArrayD *GetLeft () { return fChannelL; } 
    TArrayD *GetRight() { return fChannelR; } 
    TArrayD *GetBelow() { return fChannelL; } 
    TArrayD *GetAbove() { return fChannelR; } 

    TF1  *GetFuncPulse() { return fFuncPulse; }
    TH1D *GetHistPulse() { return fHistPulse; }

    Double_t PulseWithError(double *xx, double *pp);

  ClassDef(STChannelBar, 5);
};

#endif
