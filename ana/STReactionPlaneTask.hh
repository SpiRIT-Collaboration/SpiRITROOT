//-----------------------------------------------------------
// Description:
//   Embed pulses onto the data
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//   Tommy Tsang     MSU                  (decouple this class from STDecoder class)
//-----------------------------------------------------------

#ifndef _STREACTIONPLANETASK_H_
#define _STREACTIONPLANETASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "STAnaParticleDB.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TH2.h"
#include "TH1.h"
#include "TParameter.h"

// STL
#include <vector>
#include <memory>



class STReactionPlaneTask : public FairTask {
  public:
    STReactionPlaneTask();
    /// Destructor
    ~STReactionPlaneTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();

    void SetPersistence(Bool_t value);
    void LoadBiasCorrection(const std::string& bias_filename);
    void SetChargeCoef(double coef) { fChargeCoef = coef; }
    void SetMassCoef(double coef) { fMassCoef = coef; }
    void SetConstCoef(double coef) { fConstCoef = coef; }
    void SetParticleCoef(const std::vector<double>& val) { fParticleCoef = val; }
    void UseMCReactionPlane(bool val = true, double res = 0) { fUseMCReactionPlane = val; fMCReactionPlaneRes = res*TMath::DegToRad(); }
    void SetMaxTheta(double val) { fThetaCut = val; }
    void SetMidRapidity(double val) { fMidRapidity = val; }
    void SetPIDCut(double prob, double sd, double dpoca) { fProbVal = prob; fSDVal = sd; fDPocaVal = dpoca; }
    void SetPhiEffThreshold(double threshold) { fPhiEffThreshold = threshold; }
    void SetPrepWeight(bool val) { fPrepWeight = val; }
    void UseShifting(bool val = true) { fShift = val; }
    void UseFlattening(bool val = true) {fFlat = val; }

    static void CreateBiasCorrection(const std::string& ana_filename, const std::string& out_filename);
    static double ReactionPlaneRes(const std::string& filename1, const std::string& filename2);
  private:
    double Correction(const TVector2& Q_vec);
    double Shifting(TVector2 Q_vec);
    double Flattening(double phi);
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fData;
    TClonesArray *fCMVector;
    TClonesArray *fProb;
    TClonesArray *fSD;
    TClonesArray *fFragRapidity;
    TClonesArray *fPhiEff;
    TClonesArray *fV1RPAngle;
    TClonesArray *fV2RPAngle;
    TClonesArray *fQx;
    TClonesArray *fQy;
    STVectorF *fBeamRapidity;
    STVectorF *fMCRotZ;

    STVectorF *fReactionPlane;
    STVectorF *fReactionPlaneV2;
    STVectorF *fQMag;
    double fThetaCut = 180; // max theta in populating v1 and v2 in lab frame
    double fMidRapidity = 0; // when |y| < fMidRapidity, they will not count towards reaction plane calculation
    const std::vector<int> fSupportedPDG = STAnaParticleDB::GetSupportedPDG();

    double fChargeCoef = 0;
    double fMassCoef = 0;
    double fConstCoef = 1;
    std::vector<double> fParticleCoef; // alternative to the linear combination. Will ignore charge, mass and const coef it this is set
    bool fUseMCReactionPlane = false; // if enabled, will rotate particle using the truth MC rotation angle instead of the infered reaction plane
    double fMCReactionPlaneRes = 0;
    double fProbVal = 0.9;
    double fSDVal = 3;
    double fDPocaVal = 20;
    double fPhiEffThreshold = 0.05;
    bool   fPrepWeight = true;

    std::string fBiasFilename;
    TFile *fBiasFile = nullptr;
    bool fShift = false;
    bool fFlat = false;
    TParameter<double> *fQx_mean = nullptr;
    TParameter<double> *fQy_mean = nullptr;
    TParameter<double> *fQx_sigma = nullptr;
    TParameter<double> *fQy_sigma = nullptr;

    STVectorF *fAn = nullptr;
    STVectorF *fBn = nullptr;
    
  ClassDef(STReactionPlaneTask, 1);
};

#endif
