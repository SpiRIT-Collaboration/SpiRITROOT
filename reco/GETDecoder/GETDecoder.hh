// =================================================
//  GETDecoder Class
// 
// 
//  Author:
//    Genie Jhang ( geniejhang@majimak.com )
//  
//  Log:
//    - 2016. 03. 23
//      MUTANT frame added
//    - 2015. 11. 09
//      Start writing new! class
//    - 2013. 09. 23
//      Start writing class
// =================================================

#ifndef GETDECODER
#define GETDECODER

#include "GETHeaderBase.hh"
#include "GETBasicFrameHeader.hh"
#include "GETLayerHeader.hh"

#include "GETTopologyFrame.hh"
#include "GETBasicFrame.hh"
#include "GETCoboFrame.hh"
#include "GETLayeredFrame.hh"
#include "GETMutantFrame.hh"

#include "GETFrameInfo.hh"

#include <fstream>
#include <vector>

#include "TROOT.h"
#include "TString.h"
#include "TClonesArray.h"

//class GETPlot;

/** Read the raw file from GET electronics and process it into GETFrame class **/
class GETDecoder
{
  public:
    //! Constructor
    GETDecoder();
    //! Constructor
    GETDecoder(TString filename /*!< GRAW filename including path */);

    void Clear(); ///< Clear data information

    //! Frame type enumerator
    enum EFrameType { kBasic, kCobo, kMergedID, kMergedTime, kMutant };

    //! Setting the number of time buckets.
    void SetNumTbs(Int_t value = 512);
    //! Add the data file to the list of rawdata.
    Bool_t AddData(TString filename);
    //! Set the data file to the class.
    Bool_t SetData(Int_t index);
    void SetDiscontinuousData(Bool_t value = kTRUE);    ///<
    //! Search the next file and set it if exists. Returns 1 if successful.
    Bool_t NextData();
    /// Set the positive signal polarity
    void SetPositivePolarity(Bool_t value = kTRUE);
    //! Print rawdata file list on the screen.
    void ShowList();
    //! Return the number of data added in the list.
    Int_t GetNumData();
    //! Return the filename of data at index
    TString GetDataName(Int_t index);

    //! Return the number of time buckets.
    Int_t GetNumTbs();
    //! Return GETPlot object pointer if there exists. If not, create a new one and return it.
    //! Return the frame type. This is used when drawing merged frame.
    EFrameType GetFrameType();

    Int_t GetNumFrames();
    //! Return specific frame of the given frame number. If **frameID** is -1, this method returns next frame.
      GETBasicFrame *GetBasicFrame(Int_t frameID = -1);
       GETCoboFrame *GetCoboFrame(Int_t frameID = -1);
    GETLayeredFrame *GetLayeredFrame(Int_t frameID = -1);
     GETMutantFrame *GetMutantFrame(Int_t frameID = -1);

    void PrintFrameInfo(Int_t frameID = -1);
    void PrintCoboFrameInfo(Int_t frameID = -1);

    //! Set the file for writing frame
    Bool_t SetWriteFile(TString filename, Bool_t overwrite = kFALSE);
    //! Write current frame
    void WriteFrame();

    //! Write metadata into ROOT file
    void SaveMetaData(TString filename = "", Int_t coboIdx = -1); 
    //! Load metadata from ROOT file
    void LoadMetaData(TString filename); 

  private:
    //! Initialize variables used in the class.
    void Initialize();

    //! Check the end of file
    void CheckEndOfData();

    //! Store current frame position
    void BackupCurrentState();
    //! Restore to the previous frame position
    void RestorePreviousState();

    //! Set topology frame information manually
    void SetPseudoTopologyFrame(Int_t asadMask, Bool_t check = kFALSE);

          GETHeaderBase *fHeaderBase;
    GETBasicFrameHeader *fBasicFrameHeader;
         GETLayerHeader *fLayerHeader;

    GETTopologyFrame *fTopologyFrame;
       GETBasicFrame *fBasicFrame;
        GETCoboFrame *fCoboFrame;
     GETLayeredFrame *fLayeredFrame;
      GETMutantFrame *fMutantFrame;

    TClonesArray *fFrameInfoArray;
    TClonesArray *fCoboFrameInfoArray;
    GETFrameInfo *fFrameInfo;
    GETFrameInfo *fCoboFrameInfo;

    Int_t fNumTbs; /// the number of time buckets. It's determined when taking data and should be changed manually by user. (Default: 512)

    EFrameType fFrameType;  /// frame type. 0: normal frame, 1: event number merged, 2: event time merged
    ULong64_t fFrameSize; ///

    Bool_t fIsDataInfo;             ///< Flag for data information existance
    Bool_t fIsDoneAnalyzing;        ///< Flag for all the frame info are read
    Bool_t fIsPositivePolarity;     ///< Flag for the signal polarity
    Bool_t fIsContinuousData;       ///< Flag for continuous data set
    Bool_t fIsMetaData;             ///< Flag for checking meta data

    std::ifstream fData;            ///< Current file data stream
    ULong64_t fDataSize;            ///< Current file size
    std::vector<TString> fDataList; ///< Data file list
    Int_t fCurrentDataID;           ///< Current data file index in list

    Int_t fFrameInfoIdx;                ///< Current frame index
    Int_t fCoboFrameInfoIdx;            ///< Current cobo frame index
    Int_t fTargetFrameInfoIdx;          ///< Target frame or cobo frame index to return

    Char_t *fBuffer;       ///< Buffer for writing frame
    TString fWriteFile;    ///< File for writing frames

    Int_t fPrevDataID;        ///< Data ID for going back to original data
    ULong64_t fPrevPosition;  ///< Byte number for going back to original data

  ClassDef(GETDecoder, 1); /// added for making dictionary by ROOT
};

#endif
