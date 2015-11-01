//---------------------------------------------------------------------
// Description:
//      RIDF analysis code for commissioning run
//
// Author List:
//      Giordano Cerizza         MSU      (original author)
//----------------------------------------------------------------------

#ifndef _STEVENTRIDFTASK_H_
#define _STEVENTRIDFTASK_H_

// FairRoot class headers
#include "FairTask.h"
#include "FairLogger.h"

// SPiRIT class headers
#include "STEventManager.hh"
#include "STEventManagerEditor.hh"

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TNtuple.h"

// anaroot
#include "TArtEventStore.hh"
#include "TArtRawEventObject.hh"
#include "TArtRawSegmentObject.hh"
#include "TArtRawDataObject.hh"
#include "TArtRawScalerObject.hh"

class STEventRIDFTask : public FairTask
{
public:
  // Constructor and Destructor
  STEventRIDFTask();
  ~STEventRIDFTask();
  
  // Operators
  // Getters
  // Setters

  // Main methods
  virtual InitStatus Init();
  virtual void SetParContainers();
  virtual void Exec(Option_t *opt);

  void SetFilename(TString pathfile);
  void Fill_scaler_histograms();
  void SetHist_scaler_ch();
  void SetHist_scaler_val();
  void SetBeamScalersPlot(Bool_t var);

private:

  STEventManager* fEventManager;

  Bool_t fBeamPlots;
  
  TFile* rootfile;
  TTree* tree;
  TArtEventStore *estore;
  TArtRawEventObject *rawevent;

  TCanvas* fCvs_a;
  TCanvas* fCvs_b;

  TH1D* fHist_a;
  TH1D* fHist_b;
  
  ClassDef(STEventRIDFTask, 4);
};

#endif
