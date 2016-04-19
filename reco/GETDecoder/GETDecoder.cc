// =================================================
//  GETDecoder Class
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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <arpa/inet.h>

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TSystem.h"

#include "GETDecoder.hh"

#include "GETFileChecker.hh"

//#define DEBUG

ClassImp(GETDecoder);

GETDecoder::GETDecoder()
:fFrameInfoArray(NULL), fCoboFrameInfoArray(NULL), fFrameInfo(NULL), fCoboFrameInfo(NULL),
 fHeaderBase(NULL), fBasicFrameHeader(NULL), fLayerHeader(NULL),
 fTopologyFrame(NULL), fBasicFrame(NULL), fCoboFrame(NULL), fLayeredFrame(NULL),
 fMutantFrame(NULL)
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
 fTopologyFrame(NULL), fBasicFrame(NULL), fCoboFrame(NULL), fLayeredFrame(NULL),
 fMutantFrame(NULL)
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
  fIsContinuousData = kTRUE;
  fIsMetaData = kFALSE;

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

  if (     fMutantFrame == NULL) fMutantFrame = new GETMutantFrame();
  else                           fMutantFrame -> Clear();

  fPrevDataID = 0;
  fPrevPosition = 0;
}

void GETDecoder::Clear() {
  fFrameType = kBasic;

  fIsDoneAnalyzing = kFALSE;
  fIsDataInfo = kFALSE;
  fIsMetaData = kFALSE;

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
         fMutantFrame -> Clear();
  
  if (fIsContinuousData) {

#ifdef DEBUG
    std::cout << "== [GETDecoder] Discontinuous data set is set. Leave data list intact!" << std::endl;
#endif

    fDataList.clear();
  }
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
  if (!fIsContinuousData) {

#ifdef DEBUG
    std::cout << "== [GETDecoder] Discontinuous data set is set. Clear info!" << std::endl;
#endif

    Clear();
  }

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

    std::cout << "== [GETDecoder] Frame Type: ";
    switch (fHeaderBase -> GetFrameType()) {
      case GETFRAMETOPOLOGY:
        fFrameType = kCobo;
        fTopologyFrame -> Read(fData);
        std::cout << "Cobo frame (Max. 4 frames)" << std::endl;
        break;

      case GETFRAMEMERGEDBYID:
        fFrameType = kMergedID;
        std::cout << "Event ID merged frame" << std::endl;
        break;

      case GETFRAMEMERGEDBYTIME:
        fFrameType = kMergedTime;
        std::cout << "Event time merged frame" << std::endl;
        break;

      case GETFRAMEMUTANT:
        fFrameType = kMutant;
        std::cout << "MUTANT frame" << std::endl;
        break;

      default:
        fFrameType = kBasic;
        std::cout << "Basic frame" << std::endl;
        break;
    }

    fIsDataInfo = kTRUE;
  } else {
    fHeaderBase -> Read(fData, kTRUE);

    if (fHeaderBase -> GetFrameType() == GETFRAMETOPOLOGY)
      fTopologyFrame -> Read(fData);
  }

  fCurrentDataID = index;

  return kTRUE;
}

void GETDecoder::SetDiscontinuousData(Bool_t value) { fIsContinuousData = !value; }
Bool_t GETDecoder::NextData() { if (fIsContinuousData) return SetData(fCurrentDataID + 1); else return kFALSE; }
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
       case kMutant:
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
  if (frameID == -1)
    fTargetFrameInfoIdx++;
  else
    fTargetFrameInfoIdx = frameID;

  while (kTRUE) {
    fData.clear();

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
        BackupCurrentState();

        if (fFrameInfo -> GetDataID() != fCurrentDataID)
          SetData(fFrameInfo -> GetDataID());
   
        fData.seekg(fFrameInfo -> GetStartByte());
        fBasicFrame -> Read(fData);

        RestorePreviousState();

#ifdef DEBUG
      cout << "Returned event ID: " << fBasicFrame -> GetEventID() << endl;
#endif

        return fBasicFrame;
      } else
        fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(++fFrameInfoIdx);
    }

    ULong64_t startByte = fData.tellg();

    fBasicFrameHeader -> Read(fData);
    fData.ignore(fBasicFrameHeader -> GetFrameSkip());

    ULong64_t endByte = startByte + fBasicFrameHeader -> GetFrameSize();

    fFrameInfo -> SetDataID(fCurrentDataID);
    fFrameInfo -> SetStartByte(startByte);
    fFrameInfo -> SetEndByte(endByte);
    fFrameInfo -> SetEventID(fBasicFrameHeader -> GetEventID());

    CheckEndOfData();
  }
}

GETCoboFrame *GETDecoder::GetCoboFrame(Int_t frameID)
{
  if (frameID == -1)
    fTargetFrameInfoIdx++;
  else
    fTargetFrameInfoIdx = frameID;

  while (kTRUE) {
    fData.clear();

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

          BackupCurrentState();

          fCoboFrameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> ConstructedAt(fCoboFrameInfoIdx);

          for (Int_t iFrame = 0; iFrame < fTopologyFrame -> GetAsadMask().count(); iFrame++) {
            if (fCoboFrameInfo -> GetDataID() != fCurrentDataID)
              SetData(fCoboFrameInfo -> GetDataID());

            fData.seekg(fCoboFrameInfo -> GetStartByte());
            fCoboFrame -> ReadFrame(fData);
            fCoboFrameInfo = fCoboFrameInfo -> GetNextInfo();
          }

          RestorePreviousState();

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

    ULong64_t startByte = fData.tellg();

    fBasicFrameHeader -> Read(fData);
    fData.ignore(fBasicFrameHeader -> GetFrameSkip());

    ULong64_t endByte = startByte + fBasicFrameHeader -> GetFrameSize();

    fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(fFrameInfoIdx++);
    fFrameInfo -> SetDataID(fCurrentDataID);
    fFrameInfo -> SetStartByte(startByte);
    fFrameInfo -> SetEndByte(endByte);
    fFrameInfo -> SetEventID(fBasicFrameHeader -> GetEventID());

  //  std::cout << "fFrameInfo -> GetEndByte(): " << fFrameInfo -> GetEndByte() << " fData.tellg(): " << fData.tellg() << " fDataSize: " << fDataSize << std::endl;
    CheckEndOfData();

    if (fCoboFrameInfo -> GetNumFrames() == 0)
      fCoboFrameInfo -> Copy(fFrameInfo);
    else if (fCoboFrameInfo -> GetEventID() == fFrameInfo -> GetEventID())
      fCoboFrameInfo -> SetNextInfo(fFrameInfo); 
    else {
      Int_t iChecker = (fCoboFrameInfoIdx - 10 < 0 ? 0 : fCoboFrameInfoIdx - 10);
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
  }
}

GETLayeredFrame *GETDecoder::GetLayeredFrame(Int_t frameID)
{
  if (frameID == -1)
    fTargetFrameInfoIdx++;
  else
    fTargetFrameInfoIdx = frameID;

  while (kTRUE) {
    fData.clear();

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
        BackupCurrentState();

        if (fFrameInfo -> GetDataID() != fCurrentDataID)
          SetData(fFrameInfo -> GetDataID());

        fData.seekg(fFrameInfo -> GetStartByte());
        fLayeredFrame -> Read(fData);

         RestorePreviousState();

#ifdef DEBUG
      cout << "Returned event ID: " << fLayeredFrame -> GetEventID() << endl;
#endif

        return fLayeredFrame;
      } else
        fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(++fFrameInfoIdx);
    }

    ULong64_t startByte = fData.tellg();

    fLayerHeader -> Read(fData);
    fData.ignore(fLayerHeader -> GetFrameSkip());

    ULong64_t endByte = startByte + fLayerHeader -> GetFrameSize();

    fFrameInfo -> SetDataID(fCurrentDataID);
    fFrameInfo -> SetStartByte(startByte);
    fFrameInfo -> SetEndByte(endByte);
    switch (fFrameType) {
      case kMergedID:
        fFrameInfo -> SetEventID(fLayerHeader -> GetEventID());
        break;

      case kMergedTime:
        fFrameInfo -> SetEventTime(fLayerHeader -> GetEventTime());
        fFrameInfo -> SetDeltaT(fLayerHeader -> GetDeltaT());
        break;

      default:
        std::cerr << "== " << __func__ << " This is serious error!" << std::endl;
        break;
    }

    CheckEndOfData();
  }
}

GETMutantFrame *GETDecoder::GetMutantFrame(Int_t frameID)
{
  if (frameID == -1)
    fTargetFrameInfoIdx++;
  else
    fTargetFrameInfoIdx = frameID;

  while (kTRUE) {
    fData.clear();

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
        BackupCurrentState();

        if (fFrameInfo -> GetDataID() != fCurrentDataID)
          SetData(fFrameInfo -> GetDataID());
   
        fData.seekg(fFrameInfo -> GetStartByte());
        fMutantFrame -> Read(fData);

        RestorePreviousState();

#ifdef DEBUG
      cout << "Returned event ID: " << fMutantFrame -> GetEventNumber() << endl;
#endif

        return fMutantFrame;
      } else
        fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(++fFrameInfoIdx);
    }

    ULong64_t startByte = fData.tellg();

    fMutantFrame -> Read(fData);

    ULong64_t endByte = startByte + fMutantFrame -> GetFrameSize();

    fFrameInfo -> SetDataID(fCurrentDataID);
    fFrameInfo -> SetStartByte(startByte);
    fFrameInfo -> SetEndByte(endByte);
    fFrameInfo -> SetEventID(fMutantFrame -> GetEventNumber());

    CheckEndOfData();
  }
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
    BackupCurrentState();
    if (fCurrentDataID != 0)
      SetData(0);
      
    std::ofstream outFile(fWriteFile.Data(), std::ios::ate|std::ios::binary|std::ios::app);
    fData.seekg(0);
    fData.read(fBuffer, fTopologyFrame -> GetFrameSize());
    outFile.write(fBuffer, fTopologyFrame -> GetFrameSize());
    outFile.close();

    RestorePreviousState();

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

  BackupCurrentState();

  std::ofstream outFile(fWriteFile.Data(), std::ios::ate|std::ios::binary|std::ios::app);
  switch (fFrameType) {
    case kCobo:
      fCoboFrameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> At(fTargetFrameInfoIdx);
      do {
        if (fCurrentDataID != fCoboFrameInfo -> GetDataID())
          SetData(fCoboFrameInfo -> GetDataID());

        ULong64_t frameSize = fCoboFrameInfo -> GetEndByte() - fCoboFrameInfo -> GetStartByte();
        fData.seekg(fCoboFrameInfo -> GetStartByte());
        fData.read(fBuffer, frameSize);
        outFile.write(fBuffer, frameSize);
        fCoboFrameInfo = fCoboFrameInfo -> GetNextInfo();
      } while (fCoboFrameInfo);
      break;

    default:
      fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> At(fTargetFrameInfoIdx);

      if (fCurrentDataID != fCoboFrameInfo -> GetDataID())
        SetData(fCoboFrameInfo -> GetDataID());

      ULong64_t frameSize = fFrameInfo -> GetEndByte() - fFrameInfo -> GetStartByte();
      fData.seekg(fFrameInfo -> GetStartByte());
      fData.read(fBuffer, frameSize);
      outFile.write(fBuffer, frameSize);
      break;
  }
  outFile.close();

  RestorePreviousState();
}

void GETDecoder::CheckEndOfData() {
  if (!fIsMetaData && fFrameInfo -> GetEndByte() == fDataSize)
    if (!NextData() && !fIsDoneAnalyzing) {

#ifdef DEBUG
      std::cout << " == [GETDecoder] File ended!" << std::endl;
#endif

      fIsDoneAnalyzing = kTRUE;
      fIsMetaData = kTRUE;

      // Writing meta data will be here.
    }
}

void GETDecoder::BackupCurrentState() {
  fPrevDataID = fCurrentDataID;
  fPrevPosition = fData.tellg();
}

void GETDecoder::RestorePreviousState() {
  if (fIsDoneAnalyzing)
    return;

  if (fPrevDataID != fCurrentDataID)
    SetData(fPrevDataID);

  fData.seekg(fPrevPosition);
}


void GETDecoder::SetPseudoTopologyFrame(Int_t asadMask, Bool_t check) {
  Char_t bytes[] = { 0x40, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x07, 0x00, 0x00, (Char_t)(asadMask&0xf), 0x00, 0x00 };
  std::stringstream topology(std::string(std::begin(bytes), std::end(bytes)));

  fTopologyFrame -> Read(*((ifstream *) &topology));
  if (check)
    fTopologyFrame -> Print();
}

void GETDecoder::GoToEnd() {
  switch (fFrameType) {
    case kCobo:
      GetCoboFrame(10000000);
      break;
    case kMergedID:
    case kMergedTime:
      GetLayeredFrame(10000000);
      break;
    case kBasic:
      GetBasicFrame(10000000);
      break;
    case kMutant:
      GetMutantFrame(10000000);
      break;
    default:
      std::cout << "== [GETDecoder] Nothing to store!" << std::endl;
      break;
  }
}

void GETDecoder::SaveMetaData(Int_t runNo, TString filename, Int_t coboIdx) {
  if (filename.IsNull()) {
    TObjArray *split = fDataList.at(0).Tokenize("/");
    filename = Form("metadata/%s.meta%s.root", ((TObjString *) split -> Last()) -> String().Data(), (coboIdx == -1 ? "" : Form(".C%d", coboIdx)));
    delete split;
  }

  gSystem -> Exec(Form("mkdir -p run_%04d/metadata", runNo));
  TFile *metaFile = new TFile(Form("run_%04d/%s", runNo, filename.Data()), "recreate");

  UInt_t dataID = 0, eventID = 0, deltaT = 0;
  ULong64_t eventTime = 0, startByte = 0, endByte = 0;
  TTree *metaTree = new TTree("MetaData", Form("Meta data tree for CoBo %d", coboIdx));
  metaTree -> Branch("dataID", &dataID);
  metaTree -> Branch("eventID", &eventID);
  metaTree -> Branch("eventTime", &eventTime);
  metaTree -> Branch("deltaT", &deltaT);
  metaTree -> Branch("startByte", &startByte);
  metaTree -> Branch("endByte", &endByte);

  UInt_t numEntries = fFrameInfoArray -> GetEntries();
  for (UInt_t iEntry = 0; iEntry < numEntries; iEntry++) {
    GETFrameInfo *frameInfo = (GETFrameInfo *) fFrameInfoArray -> At(iEntry);
    dataID = frameInfo -> GetDataID();
    eventID = frameInfo -> GetEventID();
    eventTime = frameInfo -> GetEventTime();
    deltaT = frameInfo -> GetDeltaT();
    startByte = frameInfo -> GetStartByte();
    endByte = frameInfo -> GetEndByte();

    metaTree -> Fill();
  }

  metaFile -> Write();

  delete metaTree;
  delete metaFile;

  std::ofstream listFile(Form("run_%04d/metadataList.txt", runNo), std::ios::app);
  listFile << filename << endl;
  listFile.close();
}

void GETDecoder::LoadMetaData(TString filename) {
  TFile *metaFile = new TFile(filename);

  UInt_t dataID = 0, eventID = 0, deltaT = 0;
  ULong64_t eventTime = 0, startByte = 0, endByte = 0;
  TTree *metaTree = (TTree *) metaFile -> Get("MetaData");
  metaTree -> SetBranchAddress("dataID", &dataID);
  metaTree -> SetBranchAddress("eventID", &eventID);
  metaTree -> SetBranchAddress("eventTime", &eventTime);
  metaTree -> SetBranchAddress("deltaT", &deltaT);
  metaTree -> SetBranchAddress("startByte", &startByte);
  metaTree -> SetBranchAddress("endByte", &endByte);

  fFrameInfoArray -> Clear("C");
  UInt_t numEntries = metaTree -> GetEntries();
  for (Int_t iEntry = 0; iEntry < numEntries; iEntry++) {
    metaTree -> GetEntry(iEntry);
    fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> ConstructedAt(iEntry);
    fFrameInfo -> Clear();
    fFrameInfo -> SetDataID(dataID);
    fFrameInfo -> SetEventID(eventID);
    fFrameInfo -> SetEventTime(eventTime);
    fFrameInfo -> SetDeltaT(deltaT);
    fFrameInfo -> SetStartByte(startByte);
    fFrameInfo -> SetEndByte(endByte);
  }

  delete metaFile;

  if (fFrameType == kCobo) {
    fCoboFrameInfoArray -> Clear("C");
    Int_t coboFrameInfoIdx = 0;
    for (UInt_t iEntry = 0; iEntry < numEntries; iEntry++) {
      fFrameInfo = (GETFrameInfo *) fFrameInfoArray -> At(iEntry);
      fCoboFrameInfo = (GETFrameInfo *) fCoboFrameInfoArray -> ConstructedAt(coboFrameInfoIdx);

      if (fCoboFrameInfo -> GetNumFrames() == 0)
        fCoboFrameInfo -> Copy(fFrameInfo);
      else if (fCoboFrameInfo -> GetEventID() == fFrameInfo -> GetEventID())
        fCoboFrameInfo -> SetNextInfo(fFrameInfo); 
      else {
        Int_t iChecker = (coboFrameInfoIdx - 10 < 0 ? 0 : coboFrameInfoIdx - 10);
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

      if (fCoboFrameInfo -> GetNumFrames() == fTopologyFrame -> GetAsadMask().count())
        coboFrameInfoIdx++;
    }
  }

  fIsDoneAnalyzing = kTRUE;
  fIsMetaData = kTRUE;
}
