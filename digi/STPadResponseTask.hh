/**
 * @brief Calculate pad response between anode wire and cathode pad. Put
 * charge data into pads(STPad).
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail This class receives STDriftedElectron as a input and returns
 * STRawEvent as a output.
 *
 * Calculate corresponding pads and time bucket for signal in wire plane. 
 * Pad response function is calculated from Gatti distribution. This function
 * gives signal distribution in function of position. Pads are filled base on
 * this function.
 *
 * Pad with no signal is neglected from STRawEvent.
 */

#ifndef STPADRESPONSETASK
#define STPADRESPONSETASK

// Fair class header
#include "FairTask.h"
#include "FairMCEventHeader.h"

// SPiRIT-TPC class headers
#include "STDriftedElectron.hh"
#include "STDigiPar.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

// ROOT class headers
#include "TClonesArray.h"
#include "TGraph.h"
#include "TMath.h"
#include "TF1.h"

class STPadResponseTask : public FairTask
{
  public:

    STPadResponseTask();  //!< Default constructor
    ~STPadResponseTask(); //!< Destructor

    virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
    virtual void Exec(Option_t* opt); //!< Executed for each event.
    virtual void SetParContainers();  //!< Load the parameter container from the runtime database.

    void SetPersistence(Bool_t value = kTRUE);

    /**
     * Assume Gaussian like pad response function. Original pad response
     * function is a little more narror with same FWHM compare to gaussian.
     * This method will reduce accuracy in pad response function, but will give
     * much more faster calculation time (about x5) pad response. 
     *
     * This method use just one line caculation for integrating over pad range (x).
     * Original method use TGraph interpolation evaluation with 502 points 
     * calculated from original function.
     */
    void AssumeGausPRF(Bool_t val = kTRUE) { fAssumeGausPRF = val; };
    void SetElectronicsJitterFile(TString filename) { fElectronicsJitterFilename = filename.Data(); };
    void SetGainMatchingData(TString filename);

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    void InitDummy();   //!< Initialize dummy STRawEvent which contains all (dummy) pads used in TPC.
    void ReInitDummy(); //!< Initialize dummy pads content to be empty.
    /** Clone dummy STRawEvent to input STRawEvent. Empty pads are neglected when cloning. **/
    void CloneEvent();
    void InitPRF();     //!< Initialize pad response function and related parameters.

    Int_t fEventID; //!< EventID

    TClonesArray* fElectronArray;  //!< [INPUT] Array of STDriftedElectron.
    STDriftedElectron* fElectron;  //!< [INPUT] Electron data in anode wire plane.

    TClonesArray *fRawEventArray;  //!< [OUTPUT] Array of STRawEvent.
    FairMCEventHeader *fFairMCEventHeader; //!<
    STRawEvent* fRawEvent;         //!< [OUTPUT] Event data with array of STPad.

    STRawEvent* fRawEventDummy;    //!< Dummy STRawEvent.
    Bool_t fIsActivePad[108*112];  //!< Active pad flag.

    STDigiPar* fPar; //!< Base parameter container.

    Double_t PRFunction(Double_t *x, Double_t *par); //!< Pad response function
    TF1*    fPRRow;   //!< Pad response function in TF1 for row 
    TF1*    fPRLayer; //!< Pad response function in TF1 for layer 
    TGraph* fPRIRow;  //!< Integrated pad response function in TGraph for row.

    Bool_t   fAssumeGausPRF; //!< Approximate to use gaussian function for pad response function.
    Double_t fPRIPar0; //!< Parameter to use in fitting gaussian like pad response function.

    Int_t    fTBTime;       //!< One time bucket time [ns]
    Int_t    fNTbs;         //!< Number of time buckets.
    Double_t fXPadPlane;    //!< Pad plane x-axis size. [mm]
    Double_t fZPadPlane;    //!< Pad plane z-axis size. [mm]
    Double_t fPadSizeLayer; //!< Pad size through layer. [mm]
    Double_t fPadSizeRow;   //!< Pad size through row. [mm]
    Int_t    fNRows;        //!< Number of row.
    Int_t    fNLayers;      //!< Number of layer.
    Double_t fDriftVelocity; //!M electron drift velocity
   
    Double_t fTbOffset;     //!< offset for adjusting time bucket for exp.
    std::string fElectronicsJitterFilename; //!< file with electronics jitter
    std::vector<std::vector<double>> fElectronicsJitter; //!< Loaded yoffset

    /**
     * fFillRatio[type][ipad]\n
     * Fill ratio of pads depending on fill type. Three pads contain 99.3% of
     * signal portion.
     *
     * type \n
     * - 0 : wire with 1/6 across (z) the pad.
     * - 1 : wire at the center (z) of the pad 
     * - 2 : wire with 5/6 across (z) the pad.
     */
    Double_t fFillRatio[3][5];
    std::vector<std::vector<double>> fGainMatchingDataScale; //!<



    STPadResponseTask(const STPadResponseTask&);
    STPadResponseTask operator=(const STPadResponseTask&);

  ClassDef(STPadResponseTask,1);
};

#endif
