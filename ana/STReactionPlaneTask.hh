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
#include "TH1.h"

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
    void SetPersistence(Bool_t value);
    void LoadPhiEff(const std::string& eff_filename);
    void SetChargeCoef(double coef) { fChargeCoef = coef; }
    void SetMassCoef(double coef) { fMassCoef = coef; }
    void SetConstCoef(double coef) { fConstCoef = coef; }
    void SetParticleCoef(const std::vector<double>& val) { fParticleCoef = val; }
    void UseMCReactionPlane(bool val = true) { fUseMCReactionPlane = val; }
    void SetMaxTheta(double val) { fThetaCut = val; }
    void SetMidRapidity(double val) { fMidRapidity = val; }

    static void CreatePhiEffFromData(const std::string& ana_filename, const std::string& out_filename);
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fCMVector;
    TClonesArray *fProb;
    TClonesArray *fFragRapidity;
    STVectorF *fMCRotZ;

    STVectorF *fReactionPlane;
    double fThetaCut = 180; // max theta in populating v1 and v2 in lab frame
    double fMidRapidity = 0.3; // when |y| < fMidRapidity, they will not count towards reaction plane calculation
    std::string fEffFilename;
    TFile *fEffFile = nullptr;
    TH1F *fEff = nullptr;
    const std::vector<int> fSupportedPDG = STAnaParticleDB::SupportedPDG;

    double fChargeCoef = 0;
    double fMassCoef = 0;
    double fConstCoef = 1;
    std::vector<double> fParticleCoef; // alternative to the linear combination. Will ignore charge, mass and const coef it this is set
    bool fUseMCReactionPlane = false; // if enabled, will rotate particle using the truth MC rotation angle instead of the infered reaction plane
    
  ClassDef(STReactionPlaneTask, 1);
};

#endif
