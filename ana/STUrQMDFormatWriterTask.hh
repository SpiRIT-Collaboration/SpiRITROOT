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

#ifndef _STURQMDFORMATWRITERTASK_H_
#define _STURQMDFORMATWRITERTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "STAnaParticleDB.hh"
#include "STModelToLabFrameGenerator.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TH1.h"

// STL
#include <vector>
#include <memory>



class STUrQMDFormatWriterTask : public FairTask {
  public:
    STUrQMDFormatWriterTask(const std::string& output_name, const std::string& urqmd_raw="", bool simple_format=false);
    /// Destructor
    ~STUrQMDFormatWriterTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value) { fIsPersistence = value; };
    void SetBRange(double b_min, double b_max) { fBMin = b_min; fBMax = b_max; };
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    std::ofstream fOutput;
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fData;
    TClonesArray *fProb;
    TClonesArray *fCMVector;
    TClonesArray *fEventID;
    STVectorI *fSkip;
    double fBMin = -9999, fBMax = -9999;

    STUrQMDReader *fUrQMDReader = nullptr;
    TString fEventHeader;

    bool fSimpleFormat;
    const std::vector<int> fSupportedPDG = STAnaParticleDB::GetSupportedPDG();
    
  ClassDef(STUrQMDFormatWriterTask, 1);
};

#endif
