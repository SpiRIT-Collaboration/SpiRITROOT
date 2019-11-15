#ifndef STSMALLOUTPUTTASK_HH
#define STSMALLOUTPUTTASK_HH

#include <vector>
#include <string>
#include <memory>

#include "FairEventHeader.h"

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
protected:
  TClonesArray *fSTRecoTrack = nullptr;
  TClonesArray *fVATracks = nullptr;
  TClonesArray *fSTVertex = nullptr;
  TClonesArray *fBDCVertex = nullptr;
  TClonesArray *fSTEmbedTrack = nullptr;
  TClonesArray *fSTParticle = nullptr;
  TClonesArray *fVAParticle = nullptr;
  STBeamInfo *fBeamInfo = nullptr;

  
  STData fData; //!< the class of data itself 
  std::unique_ptr<TFile> fSmallOutput_; //!< File where the tree is stored
  TTree *fSmallTree_; //!< Tree itself

  int fEventID;
  int fRunID;
  int fEventType;

  ClassDef(STSmallOutputTask,1);
};


#endif
