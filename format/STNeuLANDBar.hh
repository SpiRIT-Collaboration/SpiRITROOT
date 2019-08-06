#ifndef STNEULANDBAR_HH
#define STNEULANDBAR_HH

#include "TObject.h"
#include "TVector3.h"
#include "TArrayD.h"
#include "TH1D.h"
#include "TF1.h"

#include <cmath>
#include <vector>
using namespace std;

#include "STNeuLAND.hh"

/*
 * The data container for "one neuland scintillator bar" with two input channels on end
 *
 * "localx" correspond to x for fIsAlongXNotY = 1, or y for fIsAlongXNotY = 0
 * where localx = 0 is at the center of the bar
 *
 * "length" is length from in front of the channel to the other side of the channel
 *
 * '''
 *              -------------------------------------------------------
 *              L                          C                          R
 *              -------------------------------------------------------
 *    localx :  -fBarLength/2.             0                          fBarLength/2.
 *  R length :  0                          fBarLength/2.              fBarLength
 *  L length :  fBarLenth                  fBarLength/2.              0
 *
 * '''
 */

class STNeuLANDBar : public TObject
{
  private:
    Double_t fBarLength = 2500; ///< Bar length [mm]
    Double_t fEffc = 140.; ///< effective speed of light in side the scintillator [mm/ns]
    Double_t fAttenuationLength = 1250; ///< attenuation length of light [mm]

    /// 1; if bar is laid along x axis.
    /// 0; if bar is laid along y axis.
    Bool_t fIsAlongXNotY = 1;
    TVector3 fBarCenter = TVector3(-999,-999,-999); ///< bar center position

    /// dynamic range corresponding to bar length = fBarLength / fEffc = 17.857 [ns]
    Double_t fDynamicRange = 25.; ///< [ns]
    Double_t fSamplingTime = 0.125; ///< [ns]
    Int_t fNumTDCBins = fDynamicRange/fSamplingTime; ///< number of tdc bins = 200

    Double_t fDynamicRangeOfBarLength = fBarLength / fEffc;
    Double_t fTDCAtBarLength = fDynamicRangeOfBarLength / fSamplingTime; ///< 

    Int_t fBarID = -1; ///< bar id
    Int_t fLayer = -1; ///< bar layer 0 ~ 7
    Int_t fRow = -1;   ///< bar row 0 ~ 49
    Int_t fLChannelID = -1; ///< left channel id
    Int_t fRChannelID = -1; ///< right channel id

    Bool_t fFoundHit = false; ///< true, if FindHit() method was executed.
    Double_t fThreshold = 0;  ///< [MeV]

    TArrayD fLeft;  ///<  left(below) channel buffer
    TArrayD fRight; ///< right(above) channel buffer

    Bool_t fFired[2] = {0,0}; ///< fired(found hit) flag of left(below) and right(above) signal 
    Double_t fADC[2] = {0,0}; ///< ADC of left(below) and right(above) signal 
    Double_t fQDC[2] = {0,0}; ///< ADC of left(below) and right(above) signal 
    Double_t fTDC[2] = {0,0}; ///< ADC of left(below) and right(above) signal 

    vector<Int_t> fMCIDs; ///< mc hit id array

    Int_t fPulseTDCRange = 0;   ///<! for Fill(), set from STNeuLAND
    TH1D *fHistPulse = nullptr; ///<! for Fill(), set from STNeuLAND

    TH1D *fHistLeft = nullptr;  ///<! Histogram for drawing: beam(+z) left(-x) or Beam below(-y)
    TH1D *fHistRight = nullptr; ///<! Histogram for drawing: beam(+z) right(+x) or Beam above(+y)
    TH1D *fHistSpace = nullptr; ///<! Histogram for drawing: space unit histogram

  public:
    STNeuLANDBar();

    virtual ~STNeuLANDBar() {};

    void SetBarID(
        Int_t id,
        Int_t layer,
        Int_t row,
        Int_t lid,
        Int_t rid
        )
    {
      fBarID = id;
      fLayer = layer;
      fRow = row;
      fLChannelID = lid;
      fRChannelID = rid; 
    }

    /* Set bar properties the default values are set for NeuLAND
     *
     * @param isAlongXNotY        true: bar is along x, false: bar is along y
     * @param barCenter           bar center position in local coordinate
     * @param dynamicRange        dynamic range in time [ns]
     * @param samplingTime        sampling time of one bin [ns]
     * @param barLength           bar length [mm]
     * @param effc                effective speed of light [mm/ns]
     * @param attenuationLength   attenuation length [mm]
     */
    void SetBar(
          Bool_t isAlongXNotY,
        TVector3 barCenter,
        Double_t dynamicRange = 25.,
        Double_t samplingTime = .125,
        Double_t barLength = 2500.,
        Double_t effc = 140.,
        Double_t attenuationLength = 1250.
        )
    {
      fBarLength = barLength;
      fEffc = effc;
      fAttenuationLength = attenuationLength;
      fIsAlongXNotY = isAlongXNotY;
      fBarCenter = barCenter;
      fDynamicRange = dynamicRange;
      fSamplingTime = samplingTime;

      fNumTDCBins = fDynamicRange/fSamplingTime;
      fLeft.Set(fNumTDCBins);
      fRight.Set(fNumTDCBins);

      fDynamicRangeOfBarLength = fBarLength / fEffc;
      fTDCAtBarLength = fDynamicRangeOfBarLength / fSamplingTime;
    }

    virtual void Clear(Option_t *option="");
    virtual void Print(Option_t *option="") const;
    virtual void Draw (Option_t *option="");



    void Fill(TVector3 localpos, Double_t adc, Int_t mcid = -1);
    bool FindHit(Double_t threshold, Bool_t forceFindHit = false);



    void FillToBin(Bool_t lr, Int_t bin, Double_t value); 



    Double_t PositionToLocalX(TVector3 pos) const { return (fIsAlongXNotY ? pos.X() : pos.Y()); }



    Double_t TDCToTime(Double_t tdc)       const { return tdc * fDynamicRange / fNumTDCBins; } ///< tdc -> time
    Double_t TimeToTDC(Double_t time)      const { return time / fDynamicRange * fNumTDCBins; } ///< time -> tdc

    Double_t TimeToLength(Double_t time) const { return time * fEffc; }                  ///< time -> length
    Double_t TDCToLength(Double_t tdc)   const { return TimeToLength(TDCToTime(tdc)); } ///< tdc  -> length

    Double_t TimeToLocalX(Double_t time) const { return (time * fEffc) - fBarLength/2.; }  ///< time -> localx
    Double_t TDCLToLocalX(Double_t tdc)   const { return TimeToLocalX(TDCToTime(tdc)); } ///< tdc  -> localx

    Double_t LengthToTime(Double_t length) const { return length / fEffc; }                               ///< length -> time
    Double_t LengthToTDC (Double_t length) const { return length / fEffc / fDynamicRange * fNumTDCBins; } ///< length -> tdc

    Double_t LocalXToLengthL(Double_t localx) const { return  localx + fBarLength/2.; } ///< localx -> lengthl
    Double_t LocalXToLengthR(Double_t localx) const { return -localx + fBarLength/2.; } ///< localx -> lengthr

    Double_t LocalXToTDCL(Double_t localx) const { return LengthToTDC(LocalXToLengthL(localx)); } ///< localx -> tdcl
    Double_t LocalXToTDCR(Double_t localx) const { return LengthToTDC(LocalXToLengthR(localx)); } ///< localx -> tdcr
    Double_t LocalXToTDC (Double_t localx, Bool_t isLNotR) const {
      if (isLNotR) return LengthToTDC(LocalXToLengthL(localx));
      else         return LengthToTDC(LocalXToLengthR(localx));
    }



    Double_t  GetBarLength               ()  const { return fBarLength;               }
    Double_t  GetEffc                    ()  const { return fEffc;                    }
    Double_t  GetAttenuationLength       ()  const { return fAttenuationLength;       }
    Bool_t    GetIsAlongXNotY            ()  const { return fIsAlongXNotY;            }
    TVector3  GetBarCenter               ()  const { return fBarCenter;               }
    Double_t  GetDynamicRangeOfBarLength ()  const { return fDynamicRangeOfBarLength; }
    Double_t  GetDynamicRange            ()  const { return fDynamicRange;            }
    Double_t  GetSamplingTime            ()  const { return fSamplingTime;            }
    Int_t     GetNumTDCBins              ()  const { return fNumTDCBins;              }
    Int_t     GetBarID                   ()  const { return fBarID;                   }
    Int_t     GetLayer                   ()  const { return fLayer;                   }
    Int_t     GetRow                     ()  const { return fRow;                     }
    Int_t     GetLChannelID              ()  const { return fLChannelID;              }
    Int_t     GetRChannelID              ()  const { return fRChannelID;              }
    Bool_t    GetFoundHit                ()  const { return fFoundHit;                }
    Double_t  GetThreshold               ()  const { return fThreshold;               }

    TArrayD   GetL()  const { return fLeft; }
    TArrayD   GetR()  const { return fRight; }
    TArrayD   GetB()  const { return fLeft; }
    TArrayD   GetA()  const { return fRight; }

    Bool_t   GetFired (Int_t i) const { return fFired[i]; }
    Double_t GetADC   (Int_t i) const { return fADC[i];   }
    Double_t GetQDC   (Int_t i) const { return fQDC[i];   }
    Double_t GetTDC   (Int_t i) const { return fTDC[i];   }

    Double_t GetChargeA()  const { return sqrt(fADC[0] * fADC[1] * exp(fBarLength/fAttenuationLength)); }
    Double_t GetChargeQ()  const { return sqrt(fQDC[0] * fQDC[1] * exp(fBarLength/fAttenuationLength)); }

    Double_t GetLocalXByA() const { return .5*(- fAttenuationLength * log(fADC[0]/fADC[1]) ); }
    Double_t GetLocalXByQ() const { return .5*(- fAttenuationLength * log(fQDC[0]/fQDC[1]) ); }

    Double_t GetTDCByA() const { return LocalXToTDCL(GetLocalXByA()); }
    Double_t GetTDCByQ() const { return LocalXToTDCL(GetLocalXByQ()); }
    Double_t GetTDCByT() const { return ( fTDC[0] + (fTDCAtBarLength  - fTDC[1]) ) / 2.; }

    vector<Int_t> *GetHitIDs() { return &fMCIDs; }

    TVector3 GetTDCHitPosition() const {
      auto hitPosition = fBarCenter;
      auto localx = TDCLToLocalX(GetTDCByT());
      if (fIsAlongXNotY) hitPosition.SetX(localx);
      else hitPosition.SetY(localx);
      return hitPosition;
    }

    TVector3 GetADCHitPosition() const {
      auto hitPosition = fBarCenter;
      auto localx = GetLocalXByA();
      //auto localx = TDCLToLocalX(GetTDCByA());
      if (fIsAlongXNotY) hitPosition.SetX(localx);
      else hitPosition.SetY(localx);
      return hitPosition;
    }

    TVector3 GetQDCHitPosition() const {
      auto hitPosition = fBarCenter;
      //auto localx = TDCLToLocalX(GetTDCByQ());
      auto localx = GetLocalXByQ();
      if (fIsAlongXNotY) hitPosition.SetX(localx);
      else hitPosition.SetY(localx);
      return hitPosition;
    }

  ClassDef(STNeuLANDBar, 5);
};

#endif
