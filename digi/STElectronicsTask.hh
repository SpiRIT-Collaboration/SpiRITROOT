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

#ifndef STELECTRONICSTASK
#define STELECTRONICSTASK

#include "FairTask.h"

// SPiRIT-TPC class headers
#include "STDigiPar.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

// ROOT class headers
#include "TClonesArray.h"

class STElectronicsTask : public FairTask
{
  public:

    STElectronicsTask(); //!< Default constructor
    ~STElectronicsTask(); //!< Destructor

    virtual InitStatus Init();        //!< Initiliazation of task at the beginning of a run.
    virtual void Exec(Option_t* opt); //!< Executed for each event.
    virtual void SetParContainers();  //!< Load the parameter container from the runtime database.

    void SetPersistence(Bool_t value = kTRUE);

    void SetADCConstant(Double_t val);
    void SetPulseData(TString val);
    /**
     * Set dynamic range of ADC. Unit in [Coulomb].
     * Default value is 120 fC.
     */
    void SetDynamicRange(Double_t val);
    void SetPedestalMean(Double_t val);      //!< Set pedestal mean. Default is 400 [ADC]
    void SetPedestalSigma(Double_t val);     //!< Set pedestal sigma. Default is 4 [ADC]
    void SetPedestalSubtraction(Bool_t val); //!< Set pedestal subtraction. Default is kTRUE.
    void SetSignalPolarity(Bool_t val);      //!< Set signal polarity. Default is 1(positive).

    void SetUseSaturationTemplate(Bool_t);
    void SetSaturatedPulseData(TString);	
    void SetIsKillAfterSaturation(Bool_t);
    void SetTbRange(Int_t s, Int_t e);
    void SetGainMatchingData(TString filename){ fGainMatchingData = filename; };
  

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    Int_t fEventID;

    TClonesArray *fPPEventArray;  //!< [INPUT] Array of STRawEvent.
    STRawEvent* fPPEvent;         //!< [INPUT] Input event.

    TClonesArray *fMCTrackArray;  //!< [INPUT/OUTPUT] Array of STMCTrack of embedded events
    TClonesArray *fRawEventArray; //!< [OUTPUT] Array of STRawEvent.
    STRawEvent* fRawEvent;        //!< [OUTPUT] Ouput event.

    STDigiPar* fPar; //!< Base parameter container.

    Int_t   fNTBs;       //!< Number of time buckets.
    TString fPulseFileName;
    TString fGainMatchingData = "";          ///< Gain matching data
  
    Int_t   fNBinPulser; //!< Number of bin for pulser data.
    Int_t   fNBinSaturatedPulse; //!< Number of bin for saturated pulser data.

    Bool_t	fUseSaturationTemplate;
    TString	fSaturatedPulseFileName;
    Double_t	fPulser[256]; //!< Pulser shape data.
    Double_t	fSaturatedPulse[256];

    Double_t fADCConstant;
    Double_t fADCDynamicRange;    //!< Dynamic range of ADC [Coulomb]
    Double_t fADCMax;             //!< ADC maximum value [ADC-Ch]
    Double_t fADCMaxUseable;      //!< Actual useable ADC maximum value [ADC-Ch]
    Double_t fPedestalMean;       //!< Defualt background value of ADC [ADC-Ch]
    Double_t fPedestalSigma;      //!< Defualt background sigma of ADC [ADC-Ch]
    Bool_t   fPedestalSubtracted; //!< Pedestal subtracted flag.
    Bool_t   fSignalPolarity;     //!< Polartity of signal (1: positive, 0: negative)
    Bool_t   fKillAfterSaturation;//!< Kill after saturation ?
    std::vector<std::vector<double>> fGainMatchingDataScale; //!<

    Int_t    fStartTb;
    Int_t    fEndTb;

    STElectronicsTask(const STElectronicsTask&);
    STElectronicsTask operator=(const STElectronicsTask&);

    ClassDef(STElectronicsTask,1);
};

#endif
