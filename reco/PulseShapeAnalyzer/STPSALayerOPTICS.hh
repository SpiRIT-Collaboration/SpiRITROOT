#pragma once

#include "STPSA.hh"
#include "TSpectrum.h"
#include <vector>

/**
 * @brief Data point container for OPTICS Run.
 */
class OPTICSPoint
{
  public :
    Int_t    row;
    Int_t    tb;
    Double_t eps;
    Double_t adc;
    Double_t y;

    void Set(Int_t _row, Int_t _tb, Double_t _eps = -1, Double_t _adc = -1, Double_t _y = -1)
    {
      row = _row;
      tb  = _tb;
      eps = _eps;
      adc = _adc;
      y = _y;
    }

    OPTICSPoint &operator=(OPTICSPoint right)
    { 
      row = right.row; 
      tb  = right.tb; 
      eps = right.eps; 
      adc = right.adc; 
      y = right.y;
      return *this; 
    }
};



/**
 * @brief Sorting order of epsilon in OPTICS
 */
class OPTICSSortEps
{
  public :
    Bool_t operator() (OPTICSPoint sample1, OPTICSPoint sample2) 
    { return (sample1.eps < sample2.eps); }
};




/**
 *
 */
class OPTICSSortADC
{
  public :
    Bool_t operator() (OPTICSPoint sample1, OPTICSPoint sample2) 
    { return (sample1.adc > sample2.adc); }
};



/**
 * @brief OPTICSPoint status
 */
class OPTICSPointStatus
{
  public :
    //! Status
    enum OPTICSPointSort
    {
      kQueue,
      kCore,
      kNoise
    };

    //! Status
    OPTICSPointSort status;
    //! Index in vector fPeakPointArray -1 if point is not a peak
    Int_t idxPeak;
    //! ADC
    Double_t adc;

    void SetStatus(OPTICSPointSort _status, 
                             Int_t _idxPeak, 
                          Double_t _adc)
    {
      status = _status;
      idxPeak = _idxPeak;
      adc = _adc;
    }
};



/**
 * @brief OPTICSPoint status
 */
class STClusterizerOPTICS : public STPSA
{
  public :
    STClusterizerOPTICS();
    ~STClusterizerOPTICS();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

    void SetSearchRangeFromPeak(Double_t rowHalf, Double_t tbHalf);
    void SetThresholdADC(Double_t val);
    void SetThresholdEps(Double_t val);

    /**
     * @brief Epsilon model. Return epsilon value of current model.
     *
     * @param rowCurrent  row of current point.
     * @param tbCurrent   tb of current point.
     * @param adcCurrent  adc value of current point.
     * @param rowNb       row of comparing neighbor point.
     * @param tbNb        tb of comparing neighbor point.
     * @param adcNb       adc value of comparing neighbor point.
     */
    Double_t Eps(Double_t rowCurrent, 
                 Double_t tbCurrent, 
                 Double_t adcCurrent, 
                 Double_t rowNb, 
                 Double_t tbNb, 
                 Double_t adcNb);

  private :
    void Reset();
    void SetLayer(STRawEvent* rawEvent);
    void AnalyzeLayer(STEvent* event);
    void RunOPTICS(Int_t rowCenter, Int_t tbCenter);
    Bool_t GetNbBin(Int_t rBox, Int_t index, Int_t row0, Int_t tb0, Int_t &row, Int_t &tb);

    Int_t fCurrentLayer; 

    TSpectrum* fPeakFinder; //!< TSpectrum 

    Int_t fMinPointsForFit;
    Double_t fPercPeakMin;
    Double_t fPercPeakMax;

    Int_t fCountClusters; //!< Counting number of clusters(groups).

    void ResetCluster();
    void AddPeakToCluster(OPTICSPoint point);
    STHitCluster* GetCurrentCluster();

    Int_t fNPointsInCluster;
    TVector3 fPosCluster;
    TVector3 fSigmaCluster;
    Double_t fChargeCluster;

    Int_t fRowHalfRange; //!< half range in row direction when finding neighbors
    Int_t fTbHalfRange;  //!< half range in row direction when finding neighbors
    Int_t fNCandInRange; //!< Number of candidate neighbors in range

    Double_t fThresholdADC; //!< Adc threshold
    Double_t fThresholdEps; //!< Epsilon threshold

    // Debug eps
    TCanvas* fCvsEps;    //!< Canvas for fHistEps
    TH1D* fHistEps;      //!< n - Epsilon histotram

    TCanvas* fCvsFrame;    //!< Canvas for fHistEps
    TH2D* fHistDataSet;    //!< n - Epsilon histotram
    TGraph* fGraphCluster; //!< n - Epsilon histotram
    TMarker* fMarkerCenter; //!< center of cluster

    vector<OPTICSPoint> fPeakPointArray; //!< Array of OPTICSPoint for peak points.
    OPTICSPointStatus fStatus[108][512]; //!< Status of all bins.

  ClassDef(STPSALayerOPTICS, 1)
};
