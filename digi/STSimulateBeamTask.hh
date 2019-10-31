/**
 * @brief Simulate pulser signal made in GET electronics. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail This class receives and returns STRawEvent. 
 *
 * Pulser data is obtained from HIMAC experiment using GET electronics.
 * We use this data to geneerate signal through electronics task. Height of 
 * the output signal will be proportional to input signal magnitude.
 */ 

#ifndef STSIMULATEBEAMTASK
#define STSIMULATEBEAMTASK

#include "FairTask.h"

// SPiRIT-TPC class headers
#include "STDigiPar.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

// ROOT class headers
#include "TClonesArray.h"
#include "TGraph.h"
#include "TH2.h"
#include "TMath.h"

class STSimulateBeamTask : public FairTask
{
  public:

    STSimulateBeamTask(); //!< Default constructor
    ~STSimulateBeamTask(); //!< Destructor

    virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
    virtual void Exec(Option_t* opt); //!< Executed for each event.
    virtual void SetParContainers();  //!< Load the parameter container from the runtime database.

    void SetPersistence(Bool_t value = kTRUE);

    void SetDeadPadOnBeam(TString filename, TString treename);
    void SetHeavyFragments(TString filename, double beamheight, double fragmentStrength, double fragmentWidth);
  

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable
    TClonesArray *fPPEventArray;  //!< [INPUT] Array of STRawEvent.
    STRawEvent* fPPEvent;         //!< [INPUT] Input event.

    STDigiPar* fPar; //!< Base parameter container.
    Int_t   fNTBs;       //!< Number of time buckets.
    Double_t fPadSizeRow;
    Double_t fPadSizeLayer;
    Int_t  fNRows;
    Int_t  fNLayers;
   

    TString fDeadOnBeamProbFileName, fDeadOnBeamProbHistName; 
    TH2F    *fDeadOnBeamProb = nullptr;
    TGraph  fBeamTrajectory;
    Double_t fBeamHeight;
    Double_t fFragmentStrength;
    Double_t fFragmentWidth;
    Bool_t   fUseHeavyFragment = false;
    Int_t    fTBAtBeam;

    STSimulateBeamTask(const STSimulateBeamTask&);
    STSimulateBeamTask operator=(const STSimulateBeamTask&);

    ClassDef(STSimulateBeamTask,1);
};

#endif
