/**
 * @brief Parameter database
 */

#ifndef STPARAMETERS_HH
#define STPARAMETERS_HH

#include <map>

using std::map;

#include "TString.h"

class STParameters
{
  public:
    STParameters();
    STParameters(Int_t runID, TString systemDB, TString runDB);
    ~STParameters() {};

    void SetRunID(Int_t runID);
    void ReadSystemDB(TString inputName);
    void ReadRunDB(TString inputName);

    void CheckOk();

    Int_t GetSystemID();
    Int_t GetNumTotalEvents();
    Double_t GetSheetChargeDensity(); // ??
    Double_t GetYPedestal(); // mm
    Double_t GetBDCOffsetX(); // mm
    Double_t GetBDCOffsetY(); // mm
    Int_t GetGGRunID();
    Int_t GetRelativeGainRunID(); // V

    Double_t GetDriftVelocity(); // cm/us
    Double_t GetFieldOffsetX(); // cm
    Double_t GetFieldOffsetY(); // cm
    Double_t GetFieldOffsetZ(); // cm
    Double_t GetTargetZ(); // mm
    const Char_t *GetParameterFile(); // mm

    Int_t fRunID = -9999;

    // Run-wise : Key = Run#
    Bool_t fIsRunDBSet = kFALSE;
    map<Int_t, Int_t> fSystem;
    map<Int_t, Int_t> fTotalEvents;
    map<Int_t, Double_t> fSheetChargeDensity;
    map<Int_t, Double_t> fYPedestal;
    map<Int_t, Double_t> fBDCOffsetX;
    map<Int_t, Double_t> fBDCOffsetY;
    map<Int_t, Int_t> fRelativeGainRun;
    map<Int_t, Int_t> fGGRun;

    // System-wise : Key = SystemID (132124, 124112, 112124, 108112)
    Bool_t fIsSystemDBSet = kFALSE;
    map<Int_t, Double_t> fDriftVelocity;
    map<Int_t, Double_t> fFieldOffsetX;
    map<Int_t, Double_t> fFieldOffsetY;
    map<Int_t, Double_t> fFieldOffsetZ;
    map<Int_t, Double_t> fTargetZ;
    map<Int_t, TString> fParameterFile;

  ClassDef(STParameters, 1)
};

#endif
