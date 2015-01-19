/**
 * @brief Calculate and fill signal amount in each pad.
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail Pad response function was calculated from Gatti distribution
 * (1-dimension).
 * 
 * For efficiency in calculation time, we use integrated function F(x)
 * of pad response function for x-direction (row direction). If pad size from
 * x1 to x2, Integrated portion will be calculated as F(x2)-F(x1). This
 * Integrated function was fitted with error function which is more likely with
 * gaussian shape. But true shape is slitly different. One can use
 * SetUseIntegratedData methode to use real data points but time will take
 * longer.
 *
 * Along z-direction, there are only 3 cases in dividing signal portions
 * through pads because of wire geometry.
 * - 0 : wire is just above the border between pad and before pad. 
 * - 1 : wire is 1/3 across the pad.
 * - 2 : wire is 2/3 across the pad.
 *
 * We lose 0.7% of sinals while doing this job because we cannot go over all
 * pads.
 *
 * There are 108*112 pads in total but only active pads are stored in output.
 * So we make dummy pads which we can use for all events and copy just active
 * pads to output.
 */

#pragma once

#include "STDigiPar.hh"
#include "STGas.hh"
#include "STRawEvent.hh"
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraph.h"

class STPadResponse
{
  public :
    STPadResponse();
    ~STPadResponse() {};

    //! Initialize dummy pads content to be empty.
    void Init();
    //! Fill dummy pad with pads response signal.
    void FillPad(Int_t gain, Double_t x, Double_t t, Int_t zWire);
    /** 
     * Clone dummy STRawEvent to input STRawEvent. Empty pads are neglected
     * when cloning.
     */
    void CloneRawEvent(STRawEvent *rawEvent);
    /** 
     * Set to use pad response function integrated data points when using 
     * FillPad method. Default option is to use fitted function of data points. 
     * This option will give more precise results but will take ~ x5 longer time. 
     */
    void SetUseIntegratedData(Bool_t val = kTRUE);

  private :
    //! Initialize dummy STRawEvent which contains all (dummy) pads used in TPC.
    void InitializeRawEventDummy();
    /**
     * Find pad row, layer and filling type.
     *
     * Type number denote following description:
     * - 0 : wire is just above the border between pad and before pad. 
     * - 1 : wire is 1/3 across the pad.
     * - 2 : wire is 2/3 across the pad.
     */
    void FindPad(Double_t xElectron, 
                    Int_t zWire, 
                    Int_t &row, 
                    Int_t &layer, 
                    Int_t &type);

    STDigiPar* fPar; //!< Base parameter container.
    STGas*     fGas; //!< Gas parameter container.

    Bool_t  fUseIntegratedData; //!< Use pad response function integrated data.
    Bool_t  fDataImportFlag;    //!< Pad response function integrated data import flag.
    TGraph* fIntegratedData;    //!< Array of pad response function integrated data points. 

    STRawEvent* fRawEventDummy;   //!< Dummy STRawEvent.
    Bool_t fIsActivePad[108*112]; //!< Active pad flag.

    Double_t fTimeMax;      //!< Maximum time from primary event time.
    Int_t    fNTBs;         //!< Number of time buckets.
    Int_t    fXPadPlane;    //!< Pad plane x-axis size.
    Int_t    fZPadPlane;    //!< Pad plane z-axis size.
    Int_t    fPadSizeLayer; //!< Pad size through layer.
    Int_t    fPadSizeRow;   //!< Pad size through row.
    Int_t    fNRows;        //!< Number of row.
    Int_t    fNLayers;      //!< Number of layer.

    /**
     * fFillRatio[type][ipad]\n
     * Fill ratio of pads depending on fill type. Three pads contain 99.3% of
     * signal portion.
     */
    Double_t fFillRatio[3][3];
};
