#ifndef STSMALLOUTPUTTASK_HH
#define STSMALLOUTPUTTASK_HH

#include <vector>
#include <string>
#include <memory>

#include "FairEventHeader.h"

#include "STVector.hh"
#include "STData.hh"
#include "STRecoTask.hh"
#include "STEmbedTrack.hh"
#include "STBeamInfo.hh"

#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"
#include "TClonesArray.h"

class STSmallOutputTask : public STRecoTask
{
public:
  STSmallOutputTask();
  virtual ~STSmallOutputTask();

  void SetOutputFile(const std::string& filename);
  void SetRun(int runID);
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void FinishTask();

  void SetInPlace(bool inplace = true);
  void SetOffVerbose(Bool_t verbose = kFALSE) { fIsVerbose = verbose; }
protected:
  TClonesArray *fSTRecoTrack = nullptr;
  TClonesArray *fVATracks = nullptr;
  TClonesArray *fSTVertex = nullptr;
  TClonesArray *fBDCVertex = nullptr;
  TClonesArray *fSTEmbedTrack = nullptr;
  STBeamInfo *fBeamInfo = nullptr;
  
  STData *fData = nullptr; //!< the class of data itself 
  TClonesArray *fArrData = nullptr; //!< array of data incase data need to be stored in place
  TClonesArray *fMCEventID = nullptr;
  TClonesArray *fEventTypeArr = nullptr;
  TClonesArray *fRunIDArr = nullptr;
  std::unique_ptr<TFile> fSmallOutput_; //!< File where the tree is stored
  TTree *fSmallTree_; //!< Tree itself

  int fEventID;
  int fRunID;
  int fEventType;

  Bool_t fIsVerbose = kTRUE;
  bool fInPlace = false; // if true it will output the file into the main root file instead

  ClassDef(STSmallOutputTask,1);
};


#endif
