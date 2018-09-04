#ifndef STHITSEPARATIONTASK_HH
#define STHITSEPARATIONTASK_HH

#include "STRecoTask.hh"
#include "STPadPlaneMap.hh"

#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"

#include <vector>
using namespace std;

class STHitSeparationTask : public STRecoTask 
{
  public:
    STHitSeparationTask(Bool_t two = true);
    ~STHitSeparationTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetHitSeparationFile(TString fileName);
    void WriteHitSeparationFile();

  private:
    TClonesArray *fHitArray = nullptr;
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;

    STPadPlaneMap *fPPMapTrackHit = nullptr;

    vector<STHit *> *fTrackHits;

    TFile *fFile = nullptr;

    TTree *fFoundTree = nullptr;
    Bool_t fFound;
    Short_t fNumPad1;
    Short_t fRow;
    Short_t fLayer;
    Float_t fQ1;
    Float_t fY;

    Int_t fTrueID1;
    Int_t fTrueID2;

    Int_t fRecoID1;
    Int_t fRecoID2;

    Bool_t fTwoHitEfficiency;
    Short_t fNumPad2;
    Float_t fDTrue;
    Float_t fDReco;
    Float_t fQ2;

    Int_t fSplit = -1;
    Int_t fEventID = -1;

  ClassDef(STHitSeparationTask, 1)
};

#endif
