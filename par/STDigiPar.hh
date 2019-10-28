/**
* @brief Store parameters from ST.parameters.par for later use.
*/

#ifndef STDIGIPAR_HH
#define STDIGIPAR_HH

// FAIRROOT classes
#include "FairParGenericSet.h"
#include "FairParamList.h"
#include "FairLogger.h"

// ROOT classes
#include "TString.h"
#include "TSystem.h"

class STDigiPar : public FairParGenericSet
{
  public :
    STDigiPar(const Char_t *name, const Char_t *title, const Char_t *context);
    ~STDigiPar();

    virtual Bool_t getParams(FairParamList *paramList);
    virtual void putParams(FairParamList *paramList);
    TString GetFile(Int_t fileNum);

    Double_t GetPadPlaneX();
    Double_t GetPadPlaneZ();
    Double_t GetPadSizeX();
    Double_t GetPadSizeZ();
       Int_t GetPadRows();
       Int_t GetPadLayers();
    Double_t GetAnodeWirePlaneY();
    Double_t GetGroundWirePlaneY();
    Double_t GetGatingWirePlaneY();
    Double_t GetFPNPedestalRMS();
    Double_t GetEField();
    /// returns the number of time buckets that the data actually has
       Int_t GetNumTbs();
    /// returns the number of time buckets of the time window
       Int_t GetWindowNumTbs();
    /// returns the time bucket number of the starting point of time window
       Int_t GetWindowStartTb();
       Int_t GetSamplingRate();
    /// returns the drift length in mm
    Double_t GetDriftLength();
    /// returns the slice divider
       Int_t GetYDivider();
    Double_t GetEIonize();
    /// returns the drift velocity in cm/us.
    Double_t GetDriftVelocity();
    Double_t GetCoefDiffusionLong();
    Double_t GetCoefDiffusionTrans();
    Double_t GetCoefAttachment();
       Int_t GetGain();

     TString GetTrackingParFileName();
     TString GetUAMapFileName();
     TString GetAGETMapFileName();
     TString GetGainCalibrationDataFileName();

    Double_t GetGCConstant();
    Double_t GetGCLinear();
    Double_t GetGCQuadratic();

    /// returns the time duration of a time bucket in given sampling time in ns.
       Int_t GetTBTime();
    Bool_t IsEmbed();
    Double_t GetYDriftOffset();
    Double_t GetTotalADCWhenSat();
    void SetIsEmbed(Bool_t val);

  private :
    FairLogger *fLogger;
    Bool_t fInitialized;
    Bool_t fIsEmbed;

    Double_t fPadPlaneX;
    Double_t fPadPlaneZ;
    Double_t fPadSizeX;
    Double_t fPadSizeZ;
       Int_t fPadRows;
       Int_t fPadLayers;
    Double_t fAnodeWirePlaneY;
    Double_t fGroundWirePlaneY;
    Double_t fGatingWirePlaneY;
    Double_t fFPNPedestalRMS;
    Double_t fEField;
       Int_t fNumTbs;
       Int_t fWindowNumTbs;
       Int_t fWindowStartTb;
       Int_t fSamplingRate;
    Double_t fDriftLength;
       Int_t fYDivider;
    Double_t fEIonize;
    Double_t fDriftVelocity;
    Double_t fCoefDiffusionLong;
    Double_t fCoefDiffusionTrans;
    Double_t fCoefAttachment;
       Int_t fGain;

     TString fUAMapFileName;
       Int_t fUAMapFile;
     TString fTrackingParFileName;
       Int_t fTrackingParFile;
     TString fAGETMapFileName;
       Int_t fAGETMapFile;
     TString fGainCalibrationDataFileName;
       Int_t fGainCalibrationDataFile;

    Double_t fGCConstant;
    Double_t fGCLinear;
    Double_t fGCQuadratic;
    Double_t fYDriftOffset;
    Double_t fTotalADCWhenSat;

  ClassDef(STDigiPar, 1);
};

#endif
