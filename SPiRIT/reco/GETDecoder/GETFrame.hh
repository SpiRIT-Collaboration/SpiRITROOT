// =================================================
//  GETFrame Class
// 
//  Author:
//    Genie Jhang ( geniejhang@majimak.com )
//  
//  Log:
//    - 2013. 09. 23
//      Start writing class
// =================================================

#ifndef _GETFRAME_H_
#define _GETFRAME_H_

#include "TObject.h"

/**
  * Storage of a frame from CoBo
 **/
class GETFrame : public TObject
{
  public:
    //! Constructor
    /** Initializing the variables used in the class */
    GETFrame();

    //! Destructor
    ~GETFrame();

    //! Set the event number.
    void SetEventID(UInt_t value);
    //! Set the CoBo ID
    void SetCoboID(UShort_t value);
    //! Set the AsAd ID.
    void SetAsadID(UShort_t value);
    //! Set the frame number.
    void SetFrameID(Int_t value);

    //! Set the raw ADC **value** of the specific bucket, **buckIdx**, of the channel, **chIdx**, in the AGET, **agetIdx**.
    void SetRawADC(UShort_t agetIdx, UShort_t chIdx, UShort_t buckIdx, UShort_t value);

    //! Return the event number of this frame.
    UInt_t GetEventID();
    //! Return the CoBo ID of this frame.
    Int_t GetCoboID();
    //! Return the AsAd ID of this frame.
    Int_t GetAsadID();
    //! Return the frame number of this frame.
    Int_t GetFrameID();

    //! Return the raw ADC values array with the number of time buckets specified in GETConfig of the channel, **chIdx**, in the AGET, **agetIdx**.
    Int_t *GetRawADC(Int_t agetIdx, Int_t chIdx);
    //! Return the ADC value of the specific time bucket, **buckIdx**, of the channel, **chIdx**, in the AGET, **agetIdx**.
    Int_t GetRawADC(Int_t agetIdx, Int_t chIdx, Int_t buckIdx);

    //! Calculate pedestal and subtract it from the raw ADC value.
    void CalcPedestal(Int_t startTb = 10, //!< Starting time bucket index
                      Int_t numTbs = 20 //!< The number of time buckets from startTb in order to calculate pedestal value
                      );
    //! Return the time bucket index of the maximum ADC value.
    Int_t GetMaxADCIdx(Int_t agetIdx, Int_t chIdx);
    //! Return the pedestal-subtracted ADC values array with the number of time buckets specified in GETConfig of the channel, **chIdx**, in the AGET, **agetIdx**.
    Double_t *GetADC(Int_t agetIdx, Int_t chIdx);
    //! Return the pedestal-subtracted ADC value of the specific time bucket, **buckIdx**, of the channel, **chIdx**, in the AGET, **agetIdx**.
    Double_t GetADC(Int_t agetIdx, Int_t chIdx, Int_t buckIdx);
    //! Return the pedestal value of channel, **chIdx**, in the AGET, **agetIdx**.
    Double_t GetPedestal(Int_t agetIdx, Int_t chIdx);

  private:
    //! Internally used method to get the index of the array
    Int_t GetIndex(Int_t agetIdx, Int_t chIdx, Int_t buckIdx);

    UInt_t fEventIdx;              //!< event number
    Int_t fCoboIdx;                //!< CoBo index
    Int_t fAsadIdx;                //!< AsAd index
    Int_t fFrameIdx;               //!< frame index
    Int_t fRawAdc[4*68*512]; //!< An array containing raw ADC values

    Bool_t fPedestalSubtracted;    //!< Boolean for checking if pedestal is subtracted
    Int_t fMaxAdcIdx[4*68];        //!< An array containing indices of maximum ADC value in each channel
    Double_t fAdc[4*68*512]; //!< An array containing pedestal-subtracted ADC values
    Double_t fPedestal[4*68];      //!< An array containing pedestal value of corresponding channel

  //! Added for making dictionary by ROOT
  ClassDef(GETFrame, 1);
};

#endif
