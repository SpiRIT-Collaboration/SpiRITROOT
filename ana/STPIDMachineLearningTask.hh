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

#ifndef _STPIDMACHINELEARNINGTASK_H_
#define _STPIDMACHINELEARNINGTASK_H_

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
#include "TString.h"
#include "TH2.h"
#include "TCutG.h"
#include "TTree.h"

// STL
#include <vector>
#include <stdio.h>

class STTmpFile
{
private:
  char buffer[18];
  std::string fFileName;
public:
  STTmpFile(std::ios_base::openmode mode=std::ios::out);
  ~STTmpFile();

  void ReOpen(std::ios_base::openmode mode=std::ios::out);
  const std::string GetFileName();
  std::fstream fFile;
};

enum class STAlgorithms { NeuralNetwork, RandomForest, Voting };


class STPIDMachineLearningTask : public FairTask {
  public:
    /// Constructor
    STPIDMachineLearningTask();
    /// Destructor
    ~STPIDMachineLearningTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);
    void SetChain(TChain* chain);
    void SetID(const std::vector<int>& ids) { fTreeIDs = ids; };
    void SetBufferSize(int size);
    void SetModel(const std::string& saveModel, STAlgorithms type);

    static void TrainModel(const std::string& simulation, const std::string& saveModel, STAlgorithms type, int minClus = 0, const std::string& scaling_filename = "");
    static void dEdXMCDataRatio(const std::string& simulation, const std::string& data_to_match, const std::string& ratio_filename, bool display=false, int nevent=-1);
    static void ConvertEmbeddingConc(const std::vector<std::string>& embeddingFiles, 
                                    const std::vector<int>& particlePDG,
                                    const std::string& simulationFile);
  private:
    void LoadDataFromPython(int startID, int endID);
    static void ScaledEdX(STTmpFile& output, const std::string& simulation, const std::string& PID_scale_filename);


    FairLogger *fLogger;                ///< FairLogger singleton
    TClonesArray *fPDGProb = nullptr;
    std::vector<std::vector<double>> fPDGFromPython;
    std::map<int, STVectorF*> fPDGProbVec;
    STData *fSTData = nullptr;
    
    TTree *fChain = nullptr;
    int fBufferSize = 5000;
    std::vector<int> fTreeIDs;
    Int_t  fEventID;
    Bool_t fIsPersistence;

    bool fIsTrimmedFile = false;
    TClonesArray *fSTDataArray = nullptr; // may read from trimmed files
    std::vector<int> fSupportedPDG;

    static const std::map<int, std::string> fMLPDGToName;
    // PDG list and valid range of unsupervised learning
    static const std::map<int, int> fPIDPredictionToPDG;
    static const std::vector<std::pair<double, double>> fValidRange;

    STAlgorithms fMLType;
    std::string fSaveModel;
    TCutG fAcceptRegion;

  ClassDef(STPIDMachineLearningTask, 1);
};

                              


#endif
