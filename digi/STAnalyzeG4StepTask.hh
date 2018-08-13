#ifndef STANALYZEG4STEPTASK
#define STANALYZEG4STEPTASK

#include "FairTask.h"

#include "STDigiPar.hh"
#include "STRawEvent.hh"

#include "TClonesArray.h"
#include "TGraph.h"
#include "TF1.h"

class STAnalyzeG4StepTask : public FairTask
{
   public:
      STAnalyzeG4StepTask();
      ~STAnalyzeG4StepTask();

      virtual InitStatus Init();
      virtual void Exec(Option_t* opt);
      virtual void SetParContainers();

      void SetPersistence(Bool_t value = kTRUE);
      void SetTAPersistence(Bool_t value = kTRUE);
      void AssumeGausPRF(Bool_t val=kTRUE) { fAssumeGausPRF = val; }
      void SetGainMatchingData(Bool_t f) { fIsSetGainMatchingData = kTRUE; }

   private:
      Bool_t 	fIsPersistence;
      Bool_t 	fIsTAPersistence;
      Int_t	fEventID;

      TClonesArray* fMCPointArray;		// input TClonesArray

      TClonesArray* fRawEventArray;		// output TClonesArray
      TClonesArray* fTAMCPointArray;	// output TClonesArray

      void InitDummy();		// initialize buffer
      void ReInitDummy();	// initialize buffer to be empty
      void CloneEvent();	// copy buffer to output STRawEvent
      STRawEvent*	fRawEvent;			// output STRawEvent
      STRawEvent* fRawEventDummy;	// buffer of output STRawEvent
      Bool_t fIsActivePad[108*112];	// flag for active/inactive of pad.


      // pad response function
      void InitPRF();
      Double_t PRFunction(Double_t *x, Double_t *par);
      TF1*	fPRRow;
      TF1*	fPRLayer;
      TGraph*	fPRIRow;
      Bool_t	fAssumeGausPRF;
      Double_t fPRIPar0;

      Double_t	fFillRatio[3][5];


      // parameter container
      STDigiPar* fPar;

      // parameters for drift electron calculation.
      Double_t fYAnodeWirePlane; //!< y-position of anode wire plane. [mm] 
      Double_t fZWidthPadPlane;  //!< z-width of pad plane. 

      Int_t fNumWires;     //!< Total number of wires. 
      Int_t fZCenterWire;  //!< z-position of center wire. [mm]
      Int_t fZSpacingWire; //!< z Spacing value of wires. [mm]
      Int_t fZOffsetWire;  //!< z Offset of wires. [mm]
      Int_t fZFirstWire;   //!< z-position of first wire. (minimum z) [mm]
      Int_t fZLastWire;    //!< z-position of last wire. (maximum z) [mm]
      Int_t fZCritWire;    //!< z-position of criterion wire. (minimum positive z) [mm]
      Int_t fIFirstWire;   //!< Numbering of first wire. 
      Int_t fILastWire;    //!< Numbering of last wire.
      Int_t fICritWire;    //!< Numbering of criterion wire. (should be 0)

      Double_t fEIonize;  //!< Effective ionization energy of gas. [eV]
      Double_t fVelDrift; //!< Drift velocity of electron in gas. [mm/ns]
      Double_t fCoefT;    //!< Transversal diffusion coefficient. [mm^(-1/2)]
      Double_t fCoefL;    //!< Longitudinal diffusion coefficient. [mm^(-1/2)]
      Double_t fGain;     //!< Gain.

      // parameters for electron's pad response.
      Int_t    fTBTime;
      Int_t    fNTbs;
      Double_t fXPadPlane;
      Double_t fZPadPlane;
      Double_t fPadSizeLayer;
      Double_t fPadSizeRow;
      Int_t    fNRows;
      Int_t    fNLayers;

      Double_t fTbOffset;

      Bool_t   fIsSetGainMatchingData;
      Double_t fGainMatchingDataScale[112]={};

      STAnalyzeG4StepTask(const STAnalyzeG4StepTask&);
      STAnalyzeG4StepTask operator=(const STAnalyzeG4StepTask&);

      ClassDef(STAnalyzeG4StepTask,1);
};

#endif
