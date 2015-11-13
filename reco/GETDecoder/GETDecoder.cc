// =================================================
//  GETDecoder Class
// 
//  Author:
//    Genie Jhang ( geniejhang@majimak.com )
//  
//  Log:
//    - 2013. 09. 23
//      Start writing class
// =================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <arpa/inet.h>

#include "TString.h"

#include "GETDecoder.hh"

//#include "GETPlot.hh"
#include "GETFileChecker.hh"

#define DEBUG

ClassImp(GETDecoder);

GETDecoder::GETDecoder()
:fFrameInfoArray(NULL), fCoboFrameInfoArray(NULL), fFrameInfo(NULL), fCoboFrameInfo(NULL),
 fHeaderBase(NULL), fBasicFrameHeader(NULL), fLayerHeader(NULL),
 fTopologyFrame(NULL), fBasicFrame(NULL), fCoboFrame(NULL), fLayeredFrame(NULL)
{
  /**
    * If you use this constructor, you have to add the rawdata using
    * AddData() method and set the file with SetData() method, manually.
   **/
   
  Initialize();
}

GETDecoder::GETDecoder(TString filename)
:fFrameInfoArray(NULL), fCoboFrameInfoArray(NULL), fFrameInfo(NULL), fCoboFrameInfo(NULL),
 fHeaderBase(NULL), fBasicFrameHeader(NULL), fLayerHeader(NULL),
 fTopologyFrame(NULL), fBasicFrame(NULL), fCoboFrame(NULL), fLayeredFrame(NULL)
{
  /**
    * Automatically add the rawdata file to the list
    * and set the file to read.
   **/

  Initialize();
  AddData(filename);
  SetData(0);
}

void GETDecoder::Initialize()
{
  fNumTbs = 512;

  fFrameType = kBasic;

  fIsPositivePolarity = kFALSE;

  fIsDoneAnalyzing = kFALSE;
  fIsDataInfo = kFALSE;

  fDataSize = 0;
  fCurrentDataID = -1;

  fFrameInfoIdx = 0;
  fCoboFrameInfoIdx = 0;
  fTargetFrameInfoIdx = -1;

  fBuffer = NULL;
  fWriteFile = "";

  if (    fFrameInfoArray == NULL) fFrameInfoArray = new TClonesArray("GETFrameInfo", 10000);
  fFrameInfoArray -> Clear("C");

  if (fCoboFrameInfoArray == NULL) fCoboFrameInfoArray = new TClonesArray("GETFrameInfo", 10000);
  fCoboFrameInfoArray -> Clear("C");

  if (      fHeaderBase == NULL) fHeaderBase = new GETHeaderBase();
  else                           fHeaderBase -> Clear();

  if (fBasicFrameHeader == NULL) fBasicFrameHeader = new GETBasicFrameHeader();
  else                           fBasicFrameHeader -> Clear();

  if (     fLayerHeader == NULL) fLayerHeader = new GETLayerHeader();
  else                           fLayerHeader -> Clear();

  if (   fTopologyFrame == NULL) fTopologyFrame = new GETTopologyFrame();
  else                           fTopologyFrame -> Clear();

  if (      fBasicFrame == NULL) fBasicFrame = new GETBasicFrame();
  else                           fBasicFrame -> Clear();

  if (       fCoboFrame == NULL) fCoboFrame = new GETCoboFrame();
  else                           fCoboFrame -> Clear();

  if (    fLayeredFrame == NULL) fLayeredFrame = new GETLayeredFrame();
  else                           fLayeredFrame -> Clear();
}

void GETDecoder::Clear() {
  fFrameType = kBasic;

  fIsDoneAnalyzing = kFALSE;
  fIsDataInfo = kFALSE;

  fDataSize = 0;
  fCurrentDataID = -1;

  fFrameInfoIdx = 0;
  fCoboFrameInfoIdx = 0;
  fTargetFrameInfoIdx = -1;

  fBuffer = NULL;
  fWriteFile = "";

      fFrameInfoArray -> Clear("C");
  fCoboFrameInfoArray -> Clear("C");

          fHeaderBase -> Clear();
    fBasicFrameHeader -> Clear();
         fLayerHeader -> Clear();
       fTopologyFrame -> Clear();
          fBasicFrame -> Clear();
           fCoboFrame -> Clear();
        fLayeredFrame -> Clear();

  fDataList.clear();
}

void GETDecoder::SetNumTbs(Int_t value) { fNumTbs = value; } 

Bool_t GETDecoder::AddData(TString filename)
{
  /**
    * Check if there is a file named `filename`. If exists, add it to the list.
   **/

  TString nextData = GETFileChecker::CheckFile(filename);
  if (!nextData.EqualTo("")) {
    Bool_t isExist = 0;
    for (Int_t iIdx = 0; iIdx < fDataList.size(); iIdx++) {
      if (fDataList.at(0).EqualTo(nextData)) {
        std::cout << "== [GETDecoder] The file already exists in the list!" << std::endl;
        isExist = 1;
      }
    }

    if (!isExist) {
      fDataList.push_back(nextData);

      return kTRUE;
    }
  }

  return kFALSE;
}

Bool_t GETDecoder::SetData(Int_t index)
{
  if (index >= fDataList.size()) {
    std::cout << "== [GETDecoder] End of data list!" << std::endl;

    return kFALSE;
  }

  if (fData.is_open())
    fData.close();

  TString filename = fDataList.at(index);

  fData.open(filename.Data(), std::ios::ate|std::ios::binary);

  if (!(fData.is_open())) {
    std::cout << "== [GETDecoder] Data file open error! Check it exists!" << std::endl;

    return kFALSE;
  } 

  fDataSize = fData.tellg();

  std::cout << "== [GETDecoder] " << filename << " is opened!" << std::endl;

  fData.seekg(0);
  
  if (!fIsDataInfo) {
    fHeaderBase -> Read(fData, kTRUE);

    if (fHeaderBase -> IsBlob())
      fTopologyFrame -> Read(fData);

    std::cout << "== [GETDecoder] Frame Type: ";
    if (fTopologyFrame -> IsBlob()) {
      fFrameType = kCobo;
      std::cout << "Cobo frame (Max. 4 frames)" << std::endl;
    } else {
      fHeaderBase -> Read(fData, kTRUE);
      switch (fHeaderBase -> GetFrameType()) {
        case GETFRAMEMERGEDBYID:
          fFrameType = kMergedID;
          std::cout << "Event ID merged frame" << std::endl;
          break;

        case GETFRAMEMERGEDBYTIME:
          fFrameType = kMergedTime;
          std::cout << "Event time merged frame" << std::endl;
          break;

        default:
          fFrameType = kBasic;
          std::cout << "Basic frame" << std::endl;
          break;
      }
    }

    fIsDataInfo = kTRUE;
  }

  fCurrentDataID = index;

  return kTRUE;
}

Bool_t GETDecoder::NextData() { return SetData(fCurrentDataID + 1); }
void GETDecoder::SetPositivePolarity(Bool_t value) { fIsPositivePolarity = value; }

void GETDecoder::ShowList()
{
  std::cout << "== [GETDecoder] Index Data file" << std::endl;
  for (Int_t iItem = 0; iItem < fDataList.size(); iItem++) {
    if (iItem == fCurrentDataID)
      std::cout << " *" << std::setw(6);
    else
      std::cout << std::setw(8);

    std::cout << iItem << "  " << fDataList.at(iItem) << std::endl;
  }
}

Int_t GETDecoder::GetNumData() { return fDataList.size(); }

TString GETDecoder::GetDataName(Int_t index)
{
  if (index >= fDataList.size()) {
    std::cout << "== [GETDecoder] Size of the list is " << fDataList.size() << "!" << std::endl;

    return TString("No data!");
  }

  return fDataList.at(index);
}

Int_t GETDecoder::GetNumTbs() { return fNumTbs; }
GETDecoder::EFrameType GETDecoder::GetFrameType() { return fFrameType; }

Int_t GETDecoder::GetNumFrames() {
   if (fIsDoneAnalyzing)
     switch (fFrameType) {
       case kBasic:
       case kMergedID:
       case kMergedTime:
         return fFrameInfoArray -> GetEntriesFast(); 
         break;

       case kCobo:
         return fCoboFrameInfoArray -> GetEntriesFast();
         break;
     }

  return -1;
}

GETBasicFrame *GETDecoder::GetBasicFrame(Int_t frameID)
{
  fData.clear();

  if (frameID == -1)
    fTargetFrameInfoIdx++;
  else
    fTargetFrameInfoIdx = frameID;

  if (fIsDoneAnalyzing)
    if (fTargetFrameInfoIdx > fFrameInfoArray -> GetLast())
      return NULL;

  if (fFrameInfoIdx > fTargetFrameInfoIdx)
    fFrameInfoIdx = fTargetFrameInfoIdx;

  fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(fFrameInfoIdx);
  while (fFrameInfo -> IsFill()) {

#ifdef DEBUG
    cout << "fFrameInfoIdx: " << fFrameInfoIdx << " fTargetFrameInfoIdx: " << fTargetFrameInfoIdx << endl;
#endif

    if (fFrameInfoIdx == fTargetFrameInfoIdx) {
      if (fFrameInfo -> GetDataID() != fCurrentDataID)
        SetData(fFrameInfo -> GetDataID());

      fData.seekg(fFrameInfo -> GetStartByte());
      fBasicFrame -> Read(fData);

#ifdef DEBUG
    cout << "Returned event ID: " << fBasicFrame -> GetEventID() << endl;
#endif

      return fBasicFrame;
    } else
      fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(++fFrameInfoIdx);
  }

  fBasicFrameHeader -> Read(fData);
  fData.ignore(fBasicFrameHeader -> GetFrameSkip());

  fFrameInfo -> SetDataID(fCurrentDataID);
  fFrameInfo -> SetStartByte((ULong64_t) fData.tellg() - fBasicFrameHeader -> GetFrameSize());
  fFrameInfo -> SetEndByte(fData.tellg());
  fFrameInfo -> SetEventID(fBasicFrameHeader -> GetEventID());

  if (fFrameInfo -> GetEndByte() == fDataSize)
    if (!NextData())
      fIsDoneAnalyzing = kTRUE;

  return GetBasicFrame(fTargetFrameInfoIdx);
}

GETCoboFrame *GETDecoder::GetCoboFrame(Int_t frameID)
{
  fData.clear();

  if (frameID == -1)
    fTargetFrameInfoIdx++;
  else
    fTargetFrameInfoIdx = frameID;

  if (fIsDoneAnalyzing)
    if (fTargetFrameInfoIdx > fCoboFrameInfoArray -> GetLast())
      return NULL;

  if (fCoboFrameInfoIdx > fTargetFrameInfoIdx)
    fCoboFrameInfoIdx = fTargetFrameInfoIdx;

  fCoboFrameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> ConstructedAt(fCoboFrameInfoIdx);
  while (fCoboFrameInfo -> IsFill()) {

#ifdef DEBUG
    cout << "fFrameInfoIdx: " << fFrameInfoIdx << " fCoboFrameInfoIdx: " << fCoboFrameInfoIdx << " fTargetFrameInfoIdx: " << fTargetFrameInfoIdx << endl;
#endif

#ifdef DEBUG
    cout << "fCoboFrameInfo -> GetNumFrames(): " << fCoboFrameInfo -> GetNumFrames() << " fTopologyFrame -> GetAsadMask().count(): " << fTopologyFrame -> GetAsadMask().count() << endl;
#endif

    if (fCoboFrameInfo -> GetNumFrames() == fTopologyFrame -> GetAsadMask().count()) {
      if (fCoboFrameInfoIdx == fTargetFrameInfoIdx) {
        fCoboFrame -> Clear();

        if (fCoboFrameInfo -> GetDataID() != fCurrentDataID)
          SetData(fCoboFrameInfo -> GetDataID());

        fCoboFrameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> ConstructedAt(fCoboFrameInfoIdx);
        for (Int_t iFrame = 0; iFrame < fTopologyFrame -> GetAsadMask().count(); iFrame++) {
          fData.seekg(fCoboFrameInfo -> GetStartByte());
          fCoboFrame -> ReadFrame(fData);
          fCoboFrameInfo = fCoboFrameInfo -> GetNextInfo();
        }

#ifdef DEBUG
    cout << "Returned fCoboFrameInfoIdx: " << fCoboFrameInfoIdx << " with event ID: " << fCoboFrame -> GetFrame(0) -> GetEventID() << endl;
#endif

        return fCoboFrame;
      } else 
        fCoboFrameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> ConstructedAt(++fCoboFrameInfoIdx);
    } else
      break;
  }

#ifdef DEBUG
    cout << "Not full in fCoboFrameInfoIdx: " << fCoboFrameInfoIdx << ", reading fFrameInfoIdx: " << fFrameInfoIdx << endl;
#endif

  fBasicFrameHeader -> Read(fData);
  fData.ignore(fBasicFrameHeader -> GetFrameSkip());

  fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(fFrameInfoIdx++);
  fFrameInfo -> SetDataID(fCurrentDataID);
  fFrameInfo -> SetStartByte((ULong64_t) fData.tellg() - fBasicFrameHeader -> GetFrameSize());
  fFrameInfo -> SetEndByte(fData.tellg());
  fFrameInfo -> SetEventID(fBasicFrameHeader -> GetEventID());

  if (fFrameInfo -> GetEndByte() == fDataSize)
    if (!NextData())
      fIsDoneAnalyzing = kTRUE;

  if (fCoboFrameInfo -> GetNumFrames() == 0)
    fCoboFrameInfo -> Copy(fFrameInfo);
  else if (fCoboFrameInfo -> GetEventID() == fFrameInfo -> GetEventID())
    fCoboFrameInfo -> SetNextInfo(fFrameInfo); 
  else {
    Int_t iChecker = 0;
    while (GETFrameInfo *checkCoboFrameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> ConstructedAt(iChecker)) {
      if (checkCoboFrameInfo -> IsFill()) {
        if (checkCoboFrameInfo -> GetEventID() == fFrameInfo -> GetEventID()) {
          checkCoboFrameInfo -> SetNextInfo(fFrameInfo); 
          break;
        } else
          iChecker++;
      } else {
        checkCoboFrameInfo -> Copy(fFrameInfo);
        break;
      }
    }
  }

  return GetCoboFrame(fTargetFrameInfoIdx);
}

GETLayeredFrame *GETDecoder::GetLayeredFrame(Int_t frameID)
{
  fData.clear();

  if (frameID == -1)
    fTargetFrameInfoIdx++;
  else
    fTargetFrameInfoIdx = frameID;

  if (fIsDoneAnalyzing)
    if (fTargetFrameInfoIdx > fFrameInfoArray -> GetLast())
      return NULL;

  if (fFrameInfoIdx > fTargetFrameInfoIdx)
    fFrameInfoIdx = fTargetFrameInfoIdx;

  fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(fFrameInfoIdx);
  while (fFrameInfo -> IsFill()) {

#ifdef DEBUG
    cout << "fFrameInfoIdx: " << fFrameInfoIdx << " fTargetFrameInfoIdx: " << fTargetFrameInfoIdx << endl;
#endif

    if (fFrameInfoIdx == fTargetFrameInfoIdx) {
      if (fFrameInfo -> GetDataID() != fCurrentDataID)
        SetData(fFrameInfo -> GetDataID());

      fData.seekg(fFrameInfo -> GetStartByte());
      fLayeredFrame -> Read(fData);

#ifdef DEBUG
    cout << "Returned event ID: " << fLayeredFrame -> GetEventID() << endl;
#endif

      return fLayeredFrame;
    } else
      fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(++fFrameInfoIdx);
  }

  fLayerHeader -> Read(fData);
  fData.ignore(fLayerHeader -> GetFrameSkip());

  fFrameInfo -> SetDataID(fCurrentDataID);
  fFrameInfo -> SetStartByte((ULong64_t) fData.tellg() - fLayerHeader -> GetFrameSize());
  fFrameInfo -> SetEndByte(fData.tellg());
  switch (fFrameType) {
    case kMergedID:
      fFrameInfo -> SetEventID(fLayerHeader -> GetEventID());
      break;

    case kMergedTime:
      fFrameInfo -> SetEventTime(fLayerHeader -> GetEventTime());
      fFrameInfo -> SetDeltaT(fLayerHeader -> GetDeltaT());
      break;
  }

  if (fFrameInfo -> GetEndByte() == fDataSize)
    if (!NextData())
      fIsDoneAnalyzing = kTRUE;

  return GetLayeredFrame(fTargetFrameInfoIdx);
}

void GETDecoder::PrintFrameInfo(Int_t frameID) {
  if (frameID == -1) {
    for (Int_t iEntry = 0; iEntry < fFrameInfoArray -> GetEntriesFast(); iEntry++)
      ((GETFrameInfo *) fFrameInfoArray -> At(iEntry)) -> Print();
  } else
    ((GETFrameInfo *) fFrameInfoArray -> At(frameID)) -> Print();
}

void GETDecoder::PrintCoboFrameInfo(Int_t frameID) {
  if (frameID == -1) {
    for (Int_t iEntry = 0; iEntry < fCoboFrameInfoArray -> GetEntriesFast(); iEntry++) {
      GETFrameInfo *frameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> At(iEntry);
      do {
        frameInfo -> Print();
        frameInfo = frameInfo -> GetNextInfo();
      } while (frameInfo);
    }
  } else {
    GETFrameInfo *frameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> At(frameID);
    do {
      frameInfo -> Print();
      frameInfo = frameInfo -> GetNextInfo();
    } while (frameInfo);
  }
}

Bool_t GETDecoder::SetWriteFile(TString filename, Bool_t overwrite)
{
  fWriteFile = GETFileChecker::CheckFile(filename);
  if (!fWriteFile.IsNull() && !overwrite) {
    std::cout << "== [GETDecoder] The file you specified already exists!" << std::endl;
    std::cout << "                If you want to overwrite it, give kTRUE as a second argument." << std::endl;

    fWriteFile = "";

    return kFALSE;
  }

  fWriteFile = filename;
  std::ofstream dummy(fWriteFile.Data(), std::ios::trunc);
  dummy.close();

  if (fBuffer == NULL)
    fBuffer = new Char_t[14000000];

  if (fFrameType == kCobo) {
    Int_t currentDataID = fCurrentDataID;
    ULong64_t currentPosition = fData.tellg();
    if (fCurrentDataID != 0)
      SetData(0);
      
    std::ofstream outFile(fWriteFile.Data(), std::ios::ate|std::ios::binary|std::ios::app);
    fData.seekg(0);
    fData.read(fBuffer, fTopologyFrame -> GetFrameSize());
    outFile.write(fBuffer, fTopologyFrame -> GetFrameSize());
    outFile.close();

    if (currentDataID != 0)
      SetData(currentDataID);

    fData.seekg(currentPosition);

    std::cout << "== [GETDecoder] Topology frame is written!" << std::endl;
  }

  return kTRUE;
}

void GETDecoder::WriteFrame()
{
  if (fWriteFile.IsNull()) {
    std::cout << "== [GETDecoder] Write file is not set. Use SetWriteFile() first!" << std::endl;

    return;
  }

  std::ofstream outFile(fWriteFile.Data(), std::ios::ate|std::ios::binary|std::ios::app);
  switch (fFrameType) {
    case kCobo:
      fCoboFrameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> At(fTargetFrameInfoIdx);
      do {
        ULong64_t frameSize = fCoboFrameInfo -> GetEndByte() - fCoboFrameInfo -> GetStartByte();
        fData.seekg(fCoboFrameInfo -> GetStartByte());
        fData.read(fBuffer, frameSize);
        outFile.write(fBuffer, frameSize);
        fCoboFrameInfo = fCoboFrameInfo -> GetNextInfo();
      } while (fCoboFrameInfo);
      break;

    default:
      fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> At(fTargetFrameInfoIdx);
      ULong64_t frameSize = fFrameInfo -> GetEndByte() - fFrameInfo -> GetStartByte();
      fData.seekg(fFrameInfo -> GetStartByte());
      fData.read(fBuffer, frameSize);
      outFile.write(fBuffer, frameSize);
      break;
  }
  outFile.close();
}
